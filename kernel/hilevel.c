/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

#define MAX_PROCESSES 5

pcb_t pcb[MAX_PROCESSES];
int executing = 0;
uint32_t processes = 0;

void scheduler(ctx_t* ctx) { // priority based scheduler

  int next = 0;
  int max = 0;
  for (int i = 0; i < processes; i++) {
    if (pcb[i].priority + pcb[i].age > max && // if i has highest priorty + age then make it next
        (pcb[i].status == STATUS_READY ||
         pcb[i].status == STATUS_WAITING)) { // only accept ready or waiting processes
      max = pcb[i].priority + pcb[i].age;     // update max total priority
      next = i;                               // update next if i is the max
    }
    pcb[i].age++; // increment age of every process
  }

  pcb[next].age = 0; // reset the age of the process to be executed to 0

  if (next != executing) { // Only change processes if needed
    for (int i = 0; i < processes; i++) {
      if (i == executing) {
        if (pcb[executing].status == STATUS_EXECUTING) {     // If the current process is executing
          pcb[executing].status = STATUS_READY;              // update current process status
        }
        memcpy( &pcb[executing].ctx, ctx, sizeof( ctx_t ) ); // preserve current process
        memcpy( ctx, &pcb[next].ctx, sizeof( ctx_t ) );      // restore next process
        pcb[next].status = STATUS_EXECUTING;                 // update next process status
        executing = next;                                    // update index => next process
        break; // return early once found
      }
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
  pcb[ process ].status   = STATUS_CREATED;
  pcb[ process ].ctx.cpsr = 0x50;
  pcb[ process ].ctx.pc   = program;
  pcb[ process ].ctx.sp   = memory;
  pcb[ process ].priority = priority;
  pcb[ process ].age      = 0;
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

extern void     main_console();
extern uint32_t tos_console;
extern uint32_t tos_user;

void hilevel_handler_rst( ctx_t* ctx ) {

  for (int i = 0; i < MAX_PROCESSES; i++) {
    // right now adding 0x1000 seems to add 0x4000 instead?
    // so for now im adding 0x400 as this seems to be equivalent to 0x1000
    uint32_t memory = (uint32_t) (&tos_user + (i * 0x00000400));
    initialise_pcb( i,
                    i+1,
                    (uint32_t) (0),
                    memory,
                    0 );
  }

  initialise_pcb(0, 1, (uint32_t) (&main_console), (uint32_t) (&tos_user), 5);
  processes = 1;

  sort_pcb_by_priority(MAX_PROCESSES);

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
      // copy context of parent to child
      memcpy( &pcb[processes].ctx, ctx, sizeof( ctx_t ) );
      // set pid of child to next available pid
      pcb[processes].pid = processes + 1;
      // set r0 of child to 0 which will be the return value of fork
      pcb[processes].ctx.gpr[0] = 0;
      // set r0 of parent to the next available pid which is the pid of the child
      ctx->gpr[0] = processes + 1;

      // copy the stack of the parent process to the stack of the child process
      uint32_t* child_stack = &tos_user + (processes * 0x00001000);
      uint32_t* parent_stack = &tos_user + (executing * 0x00001000);
      memcpy( child_stack, parent_stack, 0x00001000);

      // set stack pointer of child process to correct stack pointer
      uint32_t new_sp = ctx->sp - (executing * 0x00001000) + (processes * 0x00001000);
      pcb[processes].ctx.sp = new_sp;

      // set age of child to 1000 so that it executes immediately
      // pcb[processes].age = 1000;
      // pcb[processes].priority = 1000;

      // set the status of the child process to ready
      pcb[processes].status = STATUS_READY;

      // increment the number of processes
      processes++;
      break;
    }

    case 0x04 : { // 0x04 => exit()
      pcb[ executing ].status = STATUS_TERMINATED;
      scheduler(ctx);
      break;
    }

    case 0x05 : { // 0x05 => exec()
      // set the program counter to r0 which was set to the pointer provided
      // to the exec call which points to the program to be executed
      ctx->pc = ctx->gpr[0];
      // update the stackpointer so that it is pointing at the correct stack
      // is this the correct stack though?
      ctx->sp = tos_user + (executing * 0x00001000);

      // how do i set this pcb status to executing?
      // i think its okay because i had set the status of the child to ready
      // and then the scheduler will choose it next and set the status to executing

      break;
    }

    case 0x06 : { // 0x05 => kill()
      pcb[ctx->gpr[0] - 1].status = STATUS_TERMINATED;
      // could decrement the number of processes here but then would need a way
      // of knowing which pcb is next available for fork?
      // maybe could have a loop at the beginning of fork that looks thru the
      // pcb table and finds the first pcb that is terminated or created and
      // can create the child there. this would mean that we reuse pcb slots.
      scheduler(ctx);
    }

    default : {
      break ;
    }

  }
  return;
}
