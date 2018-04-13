#include "dinner.h"

#define PHILOS 16

fork_t forks[PHILOS];

void initialise_fork(int id) {
  memset( &forks[id], 0, sizeof(fork_t));
  forks[id].id = id;
  forks[id].owner = -1;
  forks[id].dirty = true;
}

void distribute_forks() {
  for (int i = 0; i < PHILOS; i++) {
    initialise_fork(i);
    if (i < (i + 1) % PHILOS) {
      forks[i].owner = i;
    } else {
      forks[i].owner = (i + 1) % PHILOS;
    }
  }
}

void initialise_pipes(int fds[4 * PHILOS]) {
  for (int i = 0; i < (2 * PHILOS); i++) {
    int fd[2];
    pipe(fd);
    fds[i] = fd[0];
    fds[i + (2 * PHILOS)] = fd[1];
  }
}

void display_philosopher_message(int i) {
  char intbuffer[2];
  itoa(intbuffer, i);
  write(STDOUT_FILENO, "Philosopher ", 12);
  write(STDOUT_FILENO, intbuffer, 2);
  write(STDOUT_FILENO, " spawned.\n", 10);
}

void philo(int id, int left_read_fd, int left_write_fd, int right_read_fd, int right_write_fd) {
  // while(1) {
    // if (!is_eating(id)) {
    //   if (!has_left_fork(id)) {
    //     request_left_fork(id); // write to ask and then read and wait for confirmation
    //   }
    //   if (!has_right_fork(id)) {
    //     request_right_fork(id); // write to ask and then read and wait for confirmation
    //   }
    // }
  // }
}

void main_dinner() {

  int fds[4 * PHILOS];

  distribute_forks();

  initialise_pipes(fds);

  write( STDOUT_FILENO, "Dinner started...\n", 18 );
  for (int i = 0; i < 16; i++) {
    pid_t pid = fork();
    if (pid == 0) {
      display_philosopher_message(i);
      philo(i, 0, 0, 0, 0);
      exit(EXIT_SUCCESS);
    }
  }
  exit(EXIT_SUCCESS);
}
