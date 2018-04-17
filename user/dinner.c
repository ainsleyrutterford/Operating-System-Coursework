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

bool can_eat(int id) {
  char ibuff[2];
  itoa(ibuff, id);
  if (has_fork(id, "left") && has_fork(id, "right")) {
    write(STDOUT_FILENO, ibuff, 2);
    write(STDOUT_FILENO, " has both forks and can eat.\n", 29);
    return true;
  } else {
    write(STDOUT_FILENO, "", 0);
    return false;
  }
}

void eat(int id) {

  char buffer[2];
  itoa(buffer, id);
  write(STDOUT_FILENO, buffer, 2);
  write(STDOUT_FILENO, " eating...\n", 11);

  for (int i = 0; i < PHILOS; i++) {
    if (forks[i].owner == id) {
      forks[i].dirty = true;
    }
  }

}

void listen_for_request_after_eating(int id, int read_fd, int write_fd, char* side) {
  char buffer[2];
  char ibuff[2];
  itoa(ibuff, id);

  read(read_fd, buffer, 2);
  if (0 == strcmp(buffer, "rq")) {
    if (0 == strcmp(side, "right")) {
        write(STDOUT_FILENO, ibuff, 2);
        write(STDOUT_FILENO, " cleaning right fork and giving it away.\n", 42);

        clean_fork(id, "right");
        give_fork(id, "right");
    } else {
        write(STDOUT_FILENO, ibuff, 2);
        write(STDOUT_FILENO, " cleaning left fork and giving it away.\n", 41);

        clean_fork(id, "left");
        give_fork(id, "left");
    }
  }
  write(write_fd, "yy", 2);

}

void listen_for_request_with_one_fork(int id, int read_fd, int write_fd, char* side) {
  char buffer[2];
  char ibuff[2];
  itoa(ibuff, id);

  read(read_fd, buffer, 2);
  if (0 == strcmp(buffer, "rq")) {
    if (0 == strcmp(side, "left")) {
      if (fork_is_dirty(id, "left")) {
        write(STDOUT_FILENO, ibuff, 2);
        write(STDOUT_FILENO, " giving dirty left fork away.\n", 30);

        give_fork(id, "left");
        write(write_fd, "yy", 2);
      } else {
        write(STDOUT_FILENO, ibuff, 2);
        write(STDOUT_FILENO, " keeping clean left fork.\n", 26);

        write(write_fd, "nn", 2);
      }
    } else {
      if (fork_is_dirty(id, "right")) {
        write(STDOUT_FILENO, ibuff, 2);
        write(STDOUT_FILENO, " giving dirty right fork away.\n", 31);

        give_fork(id, "right");
        write(write_fd, "yy", 2);
      } else {
        write(STDOUT_FILENO, ibuff, 2);
        write(STDOUT_FILENO, " keeping clean right fork.\n", 27);

        write(write_fd, "nn", 2);
      }
    }
  }

}

void request_fork_and_eat_if_possible(int id, int left_read_fd, int left_write_fd, int right_read_fd, int right_write_fd, char* side) {
  char buffer[2];
  char ibuff[2];
  itoa(ibuff, id);

  if (0 == strcmp(side, "left")) {
    write(STDOUT_FILENO, ibuff, 2);
    write(STDOUT_FILENO, " requesting left fork.\n", 23);

    write(left_write_fd, "rq", 2);
    read(left_read_fd, buffer, 2);
    if (0 == strcmp(buffer, "yy")) {
      // The fork has been cleaned and given to us
      write(STDOUT_FILENO, ibuff, 2);
      write(STDOUT_FILENO, " obtained left fork.\n", 21);
    }
  } else {
    write(STDOUT_FILENO, ibuff, 2);
    write(STDOUT_FILENO, " requesting right fork.\n", 24);

    write(right_write_fd, "rq", 2);
    read(right_read_fd, buffer, 2);
    if (0 == strcmp(buffer, "yy")) {
      // The fork has been cleaned and given to us
      write(STDOUT_FILENO, ibuff, 2);
      write(STDOUT_FILENO, " obtained right fork.\n", 22);
    }
  }
  if (0 == strcmp(buffer, "yy")) {
    if (can_eat(id)) {
      eat(id);
      listen_for_request_after_eating(id, right_read_fd, right_write_fd, "right");
      listen_for_request_after_eating(id, left_read_fd, left_write_fd, "left");
    }
  }
}

void philo(int id, int left_read_fd, int left_write_fd, int right_read_fd, int right_write_fd) {

  display_philosopher_message(id);

  while(1) {
    char buffer[2];
    char ibuff[2];
    itoa(ibuff, id);

    if (can_eat(id)) {
      eat(id);
      listen_for_request_after_eating(id, right_read_fd, right_write_fd, "right");
      listen_for_request_after_eating(id, left_read_fd, left_write_fd, "left");
    } else {

      if (has_fork(id, "left")) {
        listen_for_request_with_one_fork(id, left_read_fd, left_write_fd, "left");
      } else {
        request_fork_and_eat_if_possible(id, left_read_fd, left_write_fd, right_read_fd, right_write_fd, "left");
      }

      if (has_fork(id, "right")) {
        listen_for_request_with_one_fork(id, right_read_fd, right_write_fd, "right");
      } else {
        request_fork_and_eat_if_possible(id, left_read_fd, left_write_fd, right_read_fd, right_write_fd, "right");
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
