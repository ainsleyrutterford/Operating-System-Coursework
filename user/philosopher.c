#include "philosopher.h"

void philosopher(int id) {
  write( STDOUT_FILENO, "philosopher \n", 12 );
  char buffer[3];
  itoa( buffer, id );
  write( STDOUT_FILENO, buffer, 3 );
  write( STDOUT_FILENO, "\n", 1 );
  exit(EXIT_SUCCESS);
}

void main_philosopher() {
  int fd[2];
  for (int i = 0; i < 16; i++) {
    pipe(fd);
    pid_t pid = fork();
    if (pid == 0) {
      philosopher(i);
    }
  }
}
