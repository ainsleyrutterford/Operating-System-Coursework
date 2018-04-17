/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

#define MAX_PROCESSES 40
#define MAX_PIPES 40
#define MAX_FDS 64
#define PIPE_FILENO 3

pcb_t pcb[MAX_PROCESSES]; int executing = 0;
pipe_t pipes[MAX_PIPES]; int next_pipe = 0;
fd_t fds[MAX_FDS]; int next_fd = 0;
uint32_t processes = 0;
bool round_robin_flag = false;
uint32_t stacks[MAX_PROCESSES];

bool switch_scheduler() {
  round_robin_flag = !round_robin_flag;
  return round_robin_flag;
}

int get_num_of_processes() {
  return processes;
}

void get_process_pids(int process_pids[processes]) {
  int n = 0;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (pcb[i].status == STATUS_READY ||
        pcb[i].status == STATUS_EXECUTING ||
        pcb[i].status == STATUS_WAITING) {
      process_pids[n] = pcb[i].pid;
      n++;
    }
  }
}

void reset_pipes() {
  next_pipe = 0;
  next_fd = 0;
}

void round_robin_scheduler(ctx_t* ctx) { // round robin scheduler
  int next = (executing + 1) % MAX_PROCESSES;
  if (processes == 1) {
    next = executing;
  } else {
    while (pcb[next].status != STATUS_READY) {
      next = (next + 1) % MAX_PROCESSES;
    }
  }
  if (next != executing) {
    if (pcb[executing].status == STATUS_EXECUTING) {     // If the current process is executing
      pcb[executing].status = STATUS_READY;              // update current process status
    }
    memcpy( &pcb[executing].ctx, ctx, sizeof( ctx_t ) ); // preserve current process
    memcpy( ctx, &pcb[next].ctx, sizeof( ctx_t ) );      // restore next process
    pcb[next].status = STATUS_EXECUTING;                 // update next process status
    executing = next;                                    // update index => next process
  }
}

void priority_based_scheduler(ctx_t* ctx) { // priority based scheduler
  int next = 0;
  int max = -1;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (pcb[i].priority + pcb[i].age > max && // if i has highest priorty + age then make it next
       (pcb[i].status == STATUS_READY)) { // only accept ready processes
      max = pcb[i].priority + pcb[i].age;     // update max total priority
      next = i;                               // update next if i is the max
    }
    if (pcb[i].status == STATUS_READY) {
      pcb[i].age++; // increment age of every process
    }
  }

  pcb[next].age = 0; // reset the age of the process to be executed to 0

  if (next != executing) { // Only change processes if needed
      if (pcb[executing].status == STATUS_EXECUTING) {     // If the current process is executing
        pcb[executing].status = STATUS_READY;              // update current process status
      }
      memcpy( &pcb[executing].ctx, ctx, sizeof( ctx_t ) ); // preserve current process
      memcpy( ctx, &pcb[next].ctx, sizeof( ctx_t ) );      // restore next process
      pcb[next].status = STATUS_EXECUTING;                 // update next process status
      executing = next;                                    // update index => next process
  }
}

void scheduler(ctx_t* ctx) {
  if (round_robin_flag) round_robin_scheduler(ctx);
  else priority_based_scheduler(ctx);
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
extern void     main_dinner();
extern uint32_t tos_user;

void hilevel_handler_rst( ctx_t* ctx ) {

  stacks[0] = (uint32_t) (&tos_user);

  for (int i = 1; i < MAX_PROCESSES; i++) {
    stacks[i] = stacks[i - 1] + 0x00001000;
    initialise_pcb( i, i+1, 0, stacks[i], 0 );
  }

  for (int i = 0; i < MAX_PIPES; i++) {
    memset(&pipes[i], 0, sizeof(pipe_t));
  }

  for (int i = 0; i < MAX_FDS; i++) {
    memset(&fds[i], 0, sizeof(fd_t));
  }

  initialise_pcb(0, 1, (uint32_t) (&main_console), (uint32_t) (&tos_user), 10);
  processes = 1;

  // initialise_pcb(0, 1, (uint32_t) (&main_philosopher), (uint32_t) (&tos_user), 5);
  // processes = 1;

  start_execution(ctx, 0);

  initialise_timer();

  int_enable_irq();

  return;
}

extern void keyboard_handler();
extern void mouse_handler();

void hilevel_handler_irq(ctx_t* ctx) {
  // Read  the interrupt identifier so we know the source.
  uint32_t id = GICC0->IAR;

  // Handle the interrupt, then clear (or reset) the source.
  if( id == GIC_SOURCE_TIMER0 ) {
    scheduler(ctx);
    TIMER0->Timer1IntClr = 0x01;
  } else if ( id == GIC_SOURCE_PS20) {
    uint8_t x = PL050_getc( PS20 );
    keyboard_handler(x);
  } else if ( id == GIC_SOURCE_PS21) {
    uint8_t x = PL050_getc( PS21 );
    mouse_handler(x);
  }

  // Write the interrupt identifier to signal we're done.
  GICC0->EOIR = id;

  return;
}

bool can_read(int fd, size_t n) {
  int p = fds[fd - PIPE_FILENO].pipe_no;
  if (n > pipes[p].size) {
    return false;
  } else {
    return true;
  }
}

void update_read_information(int fd, size_t n) {
  int p = fds[fd - 3].pipe_no;
  pcb[executing].status = STATUS_WAITING;
  pipes[p].blocking = executing;
  pipes[p].amount_blocked = n - pipes[p].size;
}

void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {
  switch(id) {

    case 0x00 : { // 0x00 => yield()
      scheduler(ctx);
      break;
    }

    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );
      char*  x = ( char* )( ctx->gpr[ 1 ] );
      int    n = ( int   )( ctx->gpr[ 2 ] );

      bool call_scheduler = false;

      if (fd == 1) {
        for( int i = 0; i < n; i++ ) {
          PL011_putc( UART0, *x++, true );
        }
      } else if (fd >= PIPE_FILENO) {
          int p = fds[fd - PIPE_FILENO].pipe_no;
          for (int i = 0; i < n; i++) {
            pipes[p].data[ (pipes[p].writeptr + i) % 100 ] = *x++; // may need to mod this with 100
          }
          pipes[p].writeptr = (pipes[p].writeptr + n) % 100; // mod 100?

          pipes[p].size = (pipes[p].size + n);
          if (pipes[p].size > 100) {
            pipes[p].size = 100; // max size a pipe can be is 100
          }

          if (pipes[p].blocking != -1) { // if this pipe is blocking anything
            pipes[p].amount_blocked -= n;
            if (pipes[p].amount_blocked <= 0) {
              pcb[pipes[p].blocking].status = STATUS_READY;
              pcb[pipes[p].blocking].age += 100; // so it will be executed next
              call_scheduler = true;
              pipes[p].blocking = -1; // now blocking nothing
            }
          }
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

      int p = fds[fd - PIPE_FILENO].pipe_no;

      for (int i = 0; i < n; i++) {
        *x++ = pipes[p].data[ (pipes[p].readptr + i) % 100 ];
      }
      pipes[p].readptr = (pipes[p].readptr + n) % 100;

      pipes[p].size -= n;

      break;
    }

    case 0x03 : { // 0x03 => fork()
      uint32_t child = processes;
      for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pcb[i].status == STATUS_CREATED || pcb[i].status == STATUS_TERMINATED) {
            child = i;
            break;
          }
        }
      // copy context of parent to child
      memcpy( &pcb[child].ctx, ctx, sizeof( ctx_t ) );
      // set pid of child to next available pid
      pcb[child].pid = child + 1;
      // set r0 of child to 0 which will be the return value of fork
      pcb[child].ctx.gpr[0] = 0;
      // set r0 of parent to the next available pid which is the pid of the child
      ctx->gpr[0] = child + 1;

      // copy the stack of the parent process to the stack of the child process
      uint32_t* child_stack = (uint32_t*) (stacks[child]);
      uint32_t* parent_stack = (uint32_t*) (stacks[executing]);
      memcpy( child_stack, parent_stack, 0x00001000);

      // set stack pointer of child process to correct stack pointer
      uint32_t new_sp = ctx->sp - (executing * 0x00001000) + (child * 0x00001000);
      pcb[child].ctx.sp = new_sp;

      // set age of child to 1000 so that it executes immediately
      pcb[child].age = 1000;

      // set the status of the child process to ready
      pcb[child].status = STATUS_READY;

      // increment the number of processes
      processes++;
      scheduler(ctx); // call scheduler so that the child process is started executing immediately
      break;
    }

    case 0x04 : { // 0x04 => exit( int x )
      initialise_pcb(executing, executing + 1, 0, stacks[executing + 1], 0);
      pcb[ executing ].status = STATUS_TERMINATED;
      scheduler(ctx);
      break;
    }

    case 0x05 : { // 0x05 => exec( const void* x )
      const void* x = ( const void* ) (ctx->gpr[ 0 ]);
      if (x != NULL) { // incase load returns NULL
        ctx->pc = ( uint32_t ) x;
        // update the stackpointer so that it is pointing at the correct stack
        ctx->sp = stacks[executing];
        // update the pcb status from ready to executing
        pcb[executing].status = STATUS_EXECUTING;
      }
      break;
    }

    case 0x06 : { // 0x06 => kill( int pid, int x )
      int pid = ( int ) (ctx->gpr[ 0 ]);
      pcb[pid - 1].status = STATUS_TERMINATED;
      processes--;
      scheduler(ctx);
      break;
    }

    case 0x07 : { // 0x07 => nice( int pid, int x )
      int pid = ( int ) (ctx->gpr[ 0 ]);
      int   x = ( int ) (ctx->gpr[ 1 ]);
      pcb[pid - 1].priority = x;
      break;
    }

    case 0x08 : { // 0x05 => pipe( void* fd )

      // ctx->gpr[ 0 ] = 0; // meant to return 0 on success but seems broken

      int* fd = ( int* ) (ctx->gpr[ 0 ]);

      memset( &pipes[next_pipe], 0, sizeof( pipe_t ) );
      pipes[next_pipe].readptr = 0;
      pipes[next_pipe].writeptr = 0;
      pipes[next_pipe].blocking = -1;
      pipes[next_pipe].amount_blocked = 0;
      pipes[next_pipe].size = 0;

      memset( &fds[next_fd], 0, sizeof( fd_t ) );
      fds[next_fd].fd      = next_fd + PIPE_FILENO;
      fds[next_fd].read    = true;
      fds[next_fd].pipe_no = next_pipe;
      fd[0] = fds[next_fd++].fd;

      memset( &fds[next_fd], 0, sizeof( fd_t ) );
      fds[next_fd].fd      = next_fd + PIPE_FILENO;
      fds[next_fd].read    = false;
      fds[next_fd].pipe_no = next_pipe;
      fd[1] = fds[next_fd++].fd;

      next_pipe++;

      break;
    }

    default : {
      break ;
    }

  }
  return;
}
