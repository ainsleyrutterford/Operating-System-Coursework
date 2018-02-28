#include "IPCtest.h"

int fd[2];

void main_IPCtest() {
  pipe(fd);
  pid_t pid = fork();
  if (pid == 0) { // child process
    write( STDOUT_FILENO, "Child process\n", 14 );
  } else { // parent process
    write( STDOUT_FILENO, "Parent process\n", 15 );
    char first[10];
    itoa( first, fd[0] );
    write( STDOUT_FILENO, first, 10 );
    char second[10];
    itoa( second, fd[1] );
    write( STDOUT_FILENO, second, 10 );
  }
  exit( EXIT_SUCCESS );
}
