/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

#define MAX_PROCESSES 20
#define MAX_PIPES 20

pcb_t pcb[MAX_PROCESSES]; int executing = 0;
pipe_t pipe[MAX_PIPES]; int next_pipe = 0;
fd_t fds[20]; int next_fd = 0;
uint32_t processes = 0;
bool round_robin_flag = false;
char data[20][100];

bool switch_scheduler() {
  round_robin_flag = !round_robin_flag;
  return round_robin_flag;
}

void round_robin_scheduler(ctx_t* ctx) { // round robin scheduler
  int next = (executing + 1) % MAX_PROCESSES;
  while (pcb[next].status != STATUS_READY) {
    next = (next + 1) % MAX_PROCESSES;
  }
  if (next != executing) {
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

void priority_based_scheduler(ctx_t* ctx) { // priority based scheduler
  int next = 0;
  int max = -1;
  for (int i = 0; i < processes; i++) {
    if (pcb[i].priority + pcb[i].age > max && // if i has highest priorty + age then make it next
       (pcb[i].status == STATUS_READY)) { // only accept ready processes
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

void scheduler(ctx_t* ctx) {
  if (round_robin_flag) round_robin_scheduler(ctx);
  else priority_based_scheduler(ctx);
}

// Insertion sort in descending order. NOT USED ATM
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
extern void     main_IPCtest();
extern void     main_philosopher();
extern uint32_t tos_user;

void hilevel_handler_rst( ctx_t* ctx ) {

  for (int i = 0; i < MAX_PROCESSES; i++) {
    // right now adding 0x1000 seems to add 0x4000 instead?
    // so for now im adding 0x400 as this seems to be equivalent to 0x1000
    uint32_t memory = (uint32_t) (&tos_user + (i * 0x00000400));
    initialise_pcb( i, i+1, (uint32_t) (0), memory, 0 );
  }

  // initialise_pcb(0, 1, (uint32_t) (&main_console), (uint32_t) (&tos_user), 10);
  // processes = 1;

  initialise_pcb(0, 1, (uint32_t) (&main_IPCtest), (uint32_t) (&tos_user), 5);
  processes = 1;

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

      bool call_scheduler = false;

      if (fd == 1) {
        for( int i = 0; i < n; i++ ) {
          PL011_putc( UART0, *x++, true );
        }
      } else if (fd > 2) {
          int p = fds[fd - 3].pipe_no;
          if (pipe[p].blocking != -1) {
            PL011_putc( UART0, 'z', true );
            pcb[pipe[p].blocking].status = STATUS_READY;
            call_scheduler = true;
          }
          for (int i = 0; i < n; i++) {
            // pipe[p].data[ (pipe[p].writeptr + i) % 100 ] = *x++; // may need to mod this with 100
            data[p][ (pipe[p].writeptr + i) % 100 ] = *x++; // may need to mod this with 100
          }
          pipe[p].writeptr = (pipe[p].writeptr + n) % 100; // mod 100?
        }

      ctx->gpr[ 0 ] = n;
      if (call_scheduler) {
        scheduler(ctx);
      }
      break;
    }

    case 0x02 : { // 0x02 => read( fd, x, n )
      int    fd = ( int   ) (ctx->gpr[ 0 ]);
      char*  x  = ( char* ) (ctx->gpr[ 1 ]);
      int    n  = ( int   ) (ctx->gpr[ 2 ]);

      int p = fds[fd - 3].pipe_no;
      if (pipe[p].blocking == executing) {
        for (int i = 0; i < n; i++) {
          // *x++ = pipe[p].data[ (pipe[p].readptr + i) % 100 ]; // mod 100?
          *x++ = data[p][ (pipe[p].readptr + i) % 100 ]; // mod 100?
        }
        pipe[p].readptr = (pipe[p].readptr + n) % 100; // mod 100?

        pipe[p].blocking = -1;
      } else {
        pcb[executing].status = STATUS_WAITING;
        pipe[p].blocking = executing;
        // ctx->pc -= 1; // might be 1. this seemed to work too?
        ctx->pc -= 12; // this seems to be the amount a read call takes.
        memcpy( &pcb[executing].ctx, ctx, sizeof( ctx_t ) );
        scheduler(ctx);
      }
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
      pcb[processes].age = 1000;

      // set the status of the child process to ready
      pcb[processes].status = STATUS_READY;

      // increment the number of processes
      processes++;
      scheduler(ctx); // call scheduler so that the child process is started executing immediately
      break;
    }

    case 0x04 : { // 0x04 => exit( int x )
      pcb[ executing ].status = STATUS_TERMINATED;
      scheduler(ctx);
      break;
    }

    case 0x05 : { // 0x05 => exec( const void* x )
      const void* x = ( const void* ) (ctx->gpr[ 0 ]);
      if (x != NULL) { // incase load returns NULL
        ctx->pc = ( uint32_t ) x;
        // update the stackpointer so that it is pointing at the correct stack
        // is this the correct stack though?
        ctx->sp = tos_user + (executing * 0x00001000);
      }

      // how do i set this pcb status to executing?
      // i think its okay because i had set the status of the child to ready
      // and then the scheduler will choose it next and set the status to executing

      break;
    }

    case 0x06 : { // 0x06 => kill( int pid, int x )
      int pid = ( int ) (ctx->gpr[ 0 ]);
      pcb[pid - 1].status = STATUS_TERMINATED;
      // could decrement the number of processes here but then would need a way
      // of knowing which pcb is next available for fork?
      // maybe could have a loop at the beginning of fork that looks thru the
      // pcb table and finds the first pcb that is terminated or created and
      // can create the child there. this would mean that we reuse pcb slots.
      scheduler(ctx);
      break;
    }

    case 0x07 : { // 0x07 => nice( int pid, int x )
      int pid = ( int ) (ctx->gpr[ 0 ]);
      int   x = ( int ) (ctx->gpr[ 1 ]);
      pcb[pid - 1].priority = x;
      // scheduler(ctx); // do i actually need to call the scheduler here?
      break;
    }

    case 0x08 : { // 0x05 => pipe( void* fd )

      // ctx->gpr[ 0 ] = 0; // meant to return 0 on success but seems broken

      int* fd = ( int* ) (ctx->gpr[ 0 ]);

      // memset( &pipe[next_pipe], 0, sizeof( pipe_t ) );
      pipe[next_pipe].readptr = 0;
      pipe[next_pipe].writeptr = 0;
      pipe[next_pipe].blocking = -1;

      fds[next_fd].fd      = next_fd + 3;
      fds[next_fd].read    = true;
      fds[next_fd].pipe_no = next_pipe;
      fd[0] = fds[next_fd].fd;

      next_fd++;

      fds[next_fd].fd      = next_fd + 3;
      fds[next_fd].read    = false;
      fds[next_fd].pipe_no = next_pipe;
      fd[1] = fds[next_fd].fd;

      next_fd++;
      next_pipe++;

      break;
    }

    default : {
      break ;
    }

  }
  return;
}
