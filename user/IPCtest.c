#include "IPCtest.h"

int fd[2];

void main_IPCtest() {
  // pipe(fd);
  pid_t pid = fork();
  if (pid == 0) { // child process
    write( STDOUT_FILENO, "Child process", 13 );
  } else { // parent process
    write( STDOUT_FILENO, "Parent process", 14 );
  }
  exit( EXIT_SUCCESS );
}
