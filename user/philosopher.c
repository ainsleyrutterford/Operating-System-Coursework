#include "philosopher.h"

void philosopher() {
  write( STDOUT_FILENO, "philosopher\n", 12 );
  exit(EXIT_SUCCESS);
}

void main_philosopher() {
  for (int i = 0; i < 16; i++) {
    pid_t pid = fork();
    if (pid == 0) {
      philosopher();
    }
  }
}
