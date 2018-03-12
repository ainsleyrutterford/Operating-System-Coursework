#include "philosopher.h"

// void philosopher(int id, int id2) {
//   write( STDOUT_FILENO, "philosopher \n", 12 );
//   char buffer[3];
//   itoa( buffer, id );
//   write( STDOUT_FILENO, buffer, 3 );
//   write( STDOUT_FILENO, "\n", 1 );
//   write( STDOUT_FILENO, "\n", 1 );
//   exit(EXIT_SUCCESS);
// }

void main_philosopher() {
  for (int i = 0; i < 16; i++) {
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    if (pid == 0) {
      // philosopher( i );
      write( STDOUT_FILENO, "philosopher ", 12 );
      char buffer[3];
      itoa( buffer, i );
      write( STDOUT_FILENO, buffer, 3 );
      write( STDOUT_FILENO, "\n", 1 );
      exit(EXIT_SUCCESS);
    }
  }
}
