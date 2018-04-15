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

int calculate_fd(char* fd, int i) {
  if (0 == strcmp(fd, "lr")) {
    return i;
  } else if (0 == strcmp(fd, "lw")) {
    return (3 * PHILOS) + ((PHILOS - 1 + i) % PHILOS);
  } else if (0 == strcmp(fd, "rr")) {
    return PHILOS + i;
  } else if (0 == strcmp(fd, "rw")) {
    return (2 * PHILOS) + ((1 + i) % PHILOS);
  } else {
    return -1;
  }
}

void philo(int id, int left_read_fd, int left_write_fd, int right_read_fd, int right_write_fd) {

  char intbuffer[2];

  // Writing "x said hi to y" to the pipe
  itoa(intbuffer, id);
  write(right_write_fd, intbuffer, 2);
  write(right_write_fd, " says hi to ", 12);

  // Reading the message from another process
  char buffer[14];
  read(left_read_fd, buffer, 14);

  // Writing the message out to stdout
  write(STDOUT_FILENO, buffer, 14);
  write(STDOUT_FILENO, intbuffer, 2);
  write(STDOUT_FILENO, "\n", 1);

  char intbuffer2[2];

  // Writing "x said hi to y" to the pipe
  itoa(intbuffer2, id);
  write(left_write_fd, intbuffer2, 2);
  write(left_write_fd, " says hi to ", 12);

  // Reading the message from another process
  char buffer2[14];
  read(right_read_fd, buffer2, 14);

  // Writing the message out to stdout
  write(STDOUT_FILENO, buffer2, 14);
  write(STDOUT_FILENO, intbuffer2, 2);
  write(STDOUT_FILENO, "\n", 1);

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
  for (int i = 0; i < PHILOS; i++) {
    int lr = fds[calculate_fd("lr", i)];
    int lw = fds[calculate_fd("lw", i)];
    int rr = fds[calculate_fd("rr", i)];
    int rw = fds[calculate_fd("rw", i)];
    pid_t pid = fork();
    if (pid == 0) {
      display_philosopher_message(i);
      philo(i, lr, lw, rr, rw);
      exit(EXIT_SUCCESS);
    }
  }
  exit(EXIT_SUCCESS);
}
