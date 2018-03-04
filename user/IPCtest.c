#include "IPCtest.h"

int fd[2];

void main_IPCtest() {
  pipe(fd);
  pid_t pid = fork();
  if (pid == 0) { // child process
    write( STDOUT_FILENO, "Child process\n", 14 );
    write( fd[1], "Hi there", 8 );
    write( fd[1], " my name is", 11 );
    write( fd[1], " Ainsley", 8 );
  } else { // parent process
    write( STDOUT_FILENO, "Parent process\n", 15 );
    char readbuffer[100];
    read( fd[0], readbuffer, 27 );
    write( STDOUT_FILENO, readbuffer, 27 );
  }
  exit( EXIT_SUCCESS );
}
