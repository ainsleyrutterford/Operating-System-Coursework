/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

#define PROCESSES 3

pcb_t pcb[ PROCESSES ]; int executing = 0;

void scheduler(ctx_t* ctx) {
  int next = (executing + 1) % PROCESSES;
  for (int i = 0; i < PROCESSES; i++) {
    if (i == executing) {
      memcpy( &pcb[ executing ].ctx, ctx, sizeof( ctx_t ) ); // preserve current process
      pcb[ executing ].status = STATUS_READY;                // update current process status
      memcpy( ctx, &pcb[ next ].ctx, sizeof( ctx_t ) );      // restore next process
      pcb[ next ].status = STATUS_EXECUTING;                 // update next process status
      executing = next;                                      // update index => next process
      break; // return early once found
    }
  }
}

void initialise_pcb(int process, uint32_t program, uint32_t memory) {
  memset( &pcb[ process ], 0, sizeof( pcb_t ) );
  pcb[ process ].pid      = (process + 1);
  pcb[ process ].status   = STATUS_READY;
  pcb[ process ].ctx.cpsr = 0x50;
  pcb[ process ].ctx.pc   = program;
  pcb[ process ].ctx.sp   = memory;
}

void start_execution(ctx_t* ctx, int process) {
  memcpy( ctx, &pcb[ process ].ctx, sizeof( ctx_t ) );
  pcb[ process ].status = STATUS_EXECUTING;
  executing = 0;
}

void initialise_timer() {
  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor
}


extern void     main_P3();
extern uint32_t tos_P3;
extern void     main_P4();
extern uint32_t tos_P4;
extern void     main_P5();
extern uint32_t tos_P5;

void hilevel_handler_rst( ctx_t* ctx ) {

  initialise_pcb(0, (uint32_t) (&main_P3), (uint32_t) (&tos_P3));
  initialise_pcb(1, (uint32_t) (&main_P4), (uint32_t) (&tos_P4));
  initialise_pcb(2, (uint32_t) (&main_P5), (uint32_t) (&tos_P5));

  start_execution(ctx, 0);

  initialise_timer();

  int_enable_irq();

  return;
}

void hilevel_handler_irq(ctx_t* ctx) {
  // Read  the interrupt identifier so we know the source.
  uint32_t id = GICC0->IAR;

  // Handle the interrupt, then clear (or reset) the source.
  if( id == GIC_SOURCE_TIMER0 ) {
    scheduler(ctx);
    TIMER0->Timer1IntClr = 0x01;
  }

  // Write the interrupt identifier to signal we're done.
  GICC0->EOIR = id;

  return;
}


void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {
  switch(id) {

    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );
      char*  x = ( char* )( ctx->gpr[ 1 ] );
      int    n = ( int   )( ctx->gpr[ 2 ] );

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++, true );
      }

      ctx->gpr[ 0 ] = n;
      break;
    }

    default : {
      break ;
    }

  }
  return;
}
