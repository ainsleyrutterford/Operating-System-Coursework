#include "IPCtest.h"

int fd[2];

void main_IPCtest() {
  pipe(fd);
  pid_t pid = fork();
  if (pid != 0) { // parent process
    write( STDOUT_FILENO, "Paren process\n", 14 );
    write( fd[1], "Hi there", 8 );
    write( fd[1], " my name is", 11 );
    write( fd[1], " Ainsley", 8 );
    // write( STDOUT_FILENO, "pa\n", 3 );
  } else { // child process
    write( STDOUT_FILENO, "Child process\n", 15 );
    char readbuffer[100];
    read( fd[0], &readbuffer, 27 );
    // write( STDOUT_FILENO, "ch\n", 3 );
    write( STDOUT_FILENO, readbuffer, 27 );
  }
  exit( EXIT_SUCCESS );
}
