#include "philosopher.h"

void philosopher(int i, int readfd, int writefd) {
  write(STDOUT_FILENO, "Philosopher ", 12);
  char intbuffer[2];
  itoa(intbuffer, i);
  write(STDOUT_FILENO, intbuffer, 2);
  write(STDOUT_FILENO, "\n", 1);
  write(writefd, intbuffer, 2);
  write(writefd, " says hi\n", 9);
  char buffer[11];
  read(readfd, buffer, 11);
  write(STDOUT_FILENO, buffer, 11);
}

int main_philosopher(int argc, char** argsv) {
  int fds[32];
  for (int i = 0; i < 16; i++) {
    int fd[2];
    pipe(fd);
    fds[i] = fd[0];
    fds[16 + i] = fd[1];
  }
  for (int i = 0; i < 16; i++) {
    int readfd = fds[i];
    int writefd = fds[32 - i];
    pid_t pid = fork();
    if (pid == 0) {
      philosopher(i, readfd, writefd);
      exit(EXIT_SUCCESS);
    }
  }
}
