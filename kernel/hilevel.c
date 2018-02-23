/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

#define PROCESSES 3

pcb_t pcb[PROCESSES]; int executing = 0;

void scheduler(ctx_t* ctx) { // round robin scheduler

  int next = (executing + 1) % PROCESSES;
  while (pcb[next].status == STATUS_TERMINATED) { // Wont this loop be infinite if there is nothing to run?
    next = (next + 1) % PROCESSES;                // how will a new process start executing if this loop is still running
  }

  for (int i = 0; i < PROCESSES; i++) {
    if (i == executing) {
      if (pcb[executing].status == STATUS_EXECUTING) {     // If the current process is executing
        pcb[executing].status = STATUS_READY;              // update current process status
      }
      memcpy( &pcb[executing].ctx, ctx, sizeof( ctx_t ) ); // preserve current process
      memcpy( ctx, &pcb[next].ctx, sizeof( ctx_t ) );      // restore next process
      pcb[next].status = STATUS_EXECUTING;                 // update next process status
      executing = next;                                      // update index => next process
      break; // return early once found
    }
  }
}

// Insertion sort in descending order.
void sort_pcb_by_priority(int n) {
  pcb_t key;
  int i;
  for (int j = 1; j < n; j++) {
    key = pcb[j];
    i = j-1;
    while (i >= 0 && pcb[i].priority < key.priority) {
      pcb[i+1] = pcb[i];
      i = i-1;
    }
    pcb[i+1] = key;
  }
}

void initialise_pcb(int process, int pid, uint32_t program, uint32_t memory, int priority) {
  memset( &pcb[ process ], 0, sizeof( pcb_t ) );
  pcb[ process ].pid      = pid;
  pcb[ process ].status   = STATUS_READY;
  pcb[ process ].ctx.cpsr = 0x50;
  pcb[ process ].ctx.pc   = program;
  pcb[ process ].ctx.sp   = memory;
  pcb[ process ].priority = priority;
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

  initialise_pcb(0, 1, (uint32_t) (&main_P3), (uint32_t) (&tos_P3), 5);
  initialise_pcb(1, 2, (uint32_t) (&main_P4), (uint32_t) (&tos_P4), 5);
  initialise_pcb(2, 3, (uint32_t) (&main_P5), (uint32_t) (&tos_P5), 10);

  sort_pcb_by_priority(PROCESSES);

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

    case 0x03 : { // 0x03 => fork()

    }

    case 0x04 : { // 0x04 => exit()
      pcb[ executing ].status = STATUS_TERMINATED;
      scheduler(ctx);
    }

    case 0x05 : { // 0x05 => exec()

    }

    default : {
      break ;
    }

  }
  return;
}
