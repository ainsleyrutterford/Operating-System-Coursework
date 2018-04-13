#include "IPCtest.h"

int fd[2];

void main_IPCtest() {
  pipe(fd);
  pid_t pid = fork();
  if (pid == 0) { // Child process
    for (int i = 0; i < 5; i++) {
      write( STDOUT_FILENO, "Child process\n", 14 );
      char readbuffer[100];
      read( fd[0], readbuffer, 28 );
      write( STDOUT_FILENO, readbuffer, 28 );
    }
  } else { // Parent process
    for (int i = 0; i < 5; i++) {
      write( STDOUT_FILENO, "Parent process\n", 15 );
      write( fd[1], "Hi there", 8 );
      write( fd[1], " my name is", 11 );
      write( fd[1], " Ainsley\n", 9 );
    }
  }
  exit( EXIT_SUCCESS );
}
