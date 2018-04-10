#include "dinner.h"

#define MAX_FORKS 16

fork_t forks[MAX_FORKS];

void initialise_fork(int id) {
  memset( &forks[id], 0, sizeof(fork_t));
  forks[id].id = id;
  forks[id].dirty = false;
}

void philosopher(int i) {
  char intbuffer[2];
  itoa(intbuffer, i);
  write(STDOUT_FILENO, "Philosopher ", 12);
  write(STDOUT_FILENO, intbuffer, 2);
  write(STDOUT_FILENO, " spawned.\n", 10);
  exit(EXIT_SUCCESS);
}

void main_dinner() {
  for (int i = 0; i < MAX_FORKS; i++) {
    initialise_fork(i);
  }
  write( STDOUT_FILENO, "Dinner started...\n", 18 );
  for (int i = 0; i < 16; i++) {
    pid_t pid = fork();
    if (pid == 0) {
      philosopher(i);
    }
  }
  write( STDOUT_FILENO, "Dinner is over.\n", 16 );
  exit(EXIT_SUCCESS);
}
