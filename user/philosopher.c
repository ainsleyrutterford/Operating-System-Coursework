#include "philosopher.h"

void philosopher(int i, int readfd, int writefd) {
  // Writing "x said hi to y" to the pipe
  char intbuffer[2];
  itoa(intbuffer, i);
  write(writefd, intbuffer, 2);
  write(writefd, " says hi to ", 12);

  // Reading the message from another process
  char buffer[14];
  read(readfd, buffer, 14);

  // Writing the message out to stdout
  write(STDOUT_FILENO, buffer, 14);
  write(STDOUT_FILENO, intbuffer, 2);
  write(STDOUT_FILENO, "\n", 1);
  write(STDOUT_FILENO, intbuffer, 2);
  write(STDOUT_FILENO, " finished\n", 10);
}

int get_partner(int i) {
  if (i < 8) {
    return i + 24;
  } else {
    return i + 8;
  }
  // 0  -> 24
  // 1  -> 25
  // 2  -> 26
  // 3  -> 27
  // 4  -> 28
  // 5  -> 29
  // 6  -> 30
  // 7  -> 31
  //
  // 8  -> 16
  // 9  -> 17
  // 10 -> 18
  // 11 -> 19
  // 12 -> 20
  // 13 -> 21
  // 14 -> 22
  // 15 -> 23
}

void main_philosopher(int argc, char** argsv) {
  int fds[32];
  for (int i = 0; i < 16; i++) {
    int fd[2];
    pipe(fd);
    fds[i] = fd[0];
    fds[16 + i] = fd[1];
  }
  for (int i = 0; i < 16; i++) {
    int readfd = fds[ i ];
    int writefd = fds[ get_partner(i) ];
    pid_t pid = fork();
    if (pid == 0) {
      philosopher(i, readfd, writefd);
      exit(EXIT_SUCCESS);
    }
  }
  write(STDOUT_FILENO, "terminating...\n", 15);
  exit(EXIT_SUCCESS);
}
