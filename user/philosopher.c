#include "philosopher.h"

void philosopher(int id, int fd1, int fd2 ) {

  write( STDOUT_FILENO, "philosopher \n", 12 );
  char buffer1[3];
  itoa( buffer1, id );
  write( STDOUT_FILENO, buffer1, 3 );
  write( STDOUT_FILENO, "\n", 1 );

  // char buffer2[3];
  // itoa( buffer2, fd1 );
  // write( STDOUT_FILENO, buffer2, 3 );
  // write( STDOUT_FILENO, "\n", 1 );
  //
  // char buffer5[3];
  // itoa( buffer5, fd2 );
  // write( STDOUT_FILENO, buffer5, 3 );
  // write( STDOUT_FILENO, "\n", 1 );

  char buffer4[100];
  write( fd2, "This is a test message", 22 );
  read( fd1, buffer4, 22 );
  write( STDOUT_FILENO, buffer4, 22 );
  write( STDOUT_FILENO, "\n", 1 );

  exit(EXIT_SUCCESS);
}

void main_philosopher() {
  for (int i = 0; i < 16; i++) {
    int fd[2];
    pipe(fd);

    // char buffer2[3];
    // itoa( buffer2, fd[0] );
    // write( STDOUT_FILENO, buffer2, 3 );
    // write( STDOUT_FILENO, "\n", 1 );
    //
    // char buffer3[3];
    // itoa( buffer3, fd[1] );
    // write( STDOUT_FILENO, buffer3, 3 );
    // write( STDOUT_FILENO, "\n", 1 );

    int fd1 = fd[0];
    int fd2 = fd[1];

    pid_t pid = fork();
    if (pid == 0) {
      philosopher( i, fd1, fd2 );
    }
  }
}
