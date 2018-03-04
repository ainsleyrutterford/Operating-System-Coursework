#include "IPCtest.h"

int fd[2];

void main_IPCtest() {
  pipe(fd);
  pid_t pid = fork();
  if (pid == 0) { // child process
    write( STDOUT_FILENO, "Child process\n", 14 );
    char readbuffer[10];
    read( fd[0], readbuffer, 8 );
    write( STDOUT_FILENO, readbuffer, 8 );
  } else { // parent process
    write( STDOUT_FILENO, "Parent process\n", 15 );
    write( fd[1], "Hi there", 8 );
  }
  exit( EXIT_SUCCESS );
}
