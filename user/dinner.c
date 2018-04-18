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

bool has_fork(int id, char* side) {
  if (0 == strcmp(side, "left")) { // Left
    if (forks[(PHILOS + id - 1) % PHILOS].owner == id) {
      return true;
    } else {
      return false;
    }
  } else { // Right
    if (forks[id].owner == id) {
      return true;
    } else {
      return false;
    }
  }
}

void give_fork(int id, char* side) {
  if (0 == strcmp(side, "left")) { // Left
    forks[(PHILOS + id - 1) % PHILOS].owner = (PHILOS + id - 1) % PHILOS;
  } else { // Right
    forks[id].owner = (id + 1) % PHILOS;
  }
}

void clean_fork(int id, char* side) {
  if (0 == strcmp(side, "left")) { // Left
    forks[(PHILOS + id - 1) % PHILOS].dirty = false;
  } else { // Right
    forks[id].dirty = false;
  }
}

bool fork_is_dirty(int id, char* side) {
  if (0 == strcmp(side, "left")) { // Left
    return forks[(PHILOS + id - 1) % PHILOS].dirty;
  } else { // Right
    return forks[id].dirty;
  }
}

void eat(int id) {

  char buffer[2];
  itoa(buffer, id);
  write(STDOUT_FILENO, buffer, 2);
  write(STDOUT_FILENO, " eating...\n", 11);

  forks[(PHILOS + id - 1) % PHILOS].dirty = false; // Dirty left fork
  forks[id].dirty = false; // Dirty right fork

}

void philo(int id, int left_read_fd, int left_write_fd, int right_read_fd, int right_write_fd) {

  display_philosopher_message(id);

  char buffer[2];
  int state = 0;

  while (1) {

    if (has_fork(id, "left") && has_fork(id, "right")) {
      state = 3;
    } else if (has_fork(id, "left") && !has_fork(id, "right")) {
      state = 2;
    } else if (!has_fork(id, "left") && has_fork(id, "right")) {
      state = 1;
    } else if (!has_fork(id, "left") && !has_fork(id, "right")) {
      state = 0;
    }

    switch (state) {
      case 3: {
        eat(id);

        read(right_read_fd, buffer, 2);
        clean_fork(id, "right");
        give_fork(id, "right");
        write(right_write_fd, "yy", 2);

        read(left_read_fd, buffer, 2);
        clean_fork(id, "left");
        give_fork(id, "left");
        write(left_write_fd, "yy", 2);
        break;
      }
      case 2: {
        read(left_read_fd, buffer, 2);
        clean_fork(id, "left");
        give_fork(id, "left");
        write(left_write_fd, "yy", 2);

        write(right_write_fd, "rq", 2);
        read(right_read_fd, buffer, 2); // should have check that you read "yy"
        break;
      }
      case 1: {
        write(left_write_fd, "rq", 2);
        read(left_read_fd, buffer, 2); // should have check that you read "yy"

        eat(id);

        read(right_read_fd, buffer, 2);
        clean_fork(id, "right");
        give_fork(id, "right");
        write(right_write_fd, "yy", 2);
        break;
      }
      case 0: {
        write(left_write_fd, "rq", 2);
        read(left_read_fd, buffer, 2); // should have check that you read "yy"

        write(right_write_fd, "rq", 2);
        read(right_read_fd, buffer, 2); // should have check that you read "yy"
        break;
      }

    }

  }

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
      philo(i, lr, lw, rr, rw);
      exit(EXIT_SUCCESS);
    }
  }
  exit(EXIT_SUCCESS);
}
