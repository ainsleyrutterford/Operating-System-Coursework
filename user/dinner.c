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
  forks[7].owner = 8;
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

extern void add_eater_dinner_gui();
extern void remove_eater_dinner_gui();
extern void add_fork_dinner_gui();
extern void remove_fork_dinner_gui();

void eat(int id) {

  char buffer[2];
  itoa(buffer, id);
  write(STDOUT_FILENO, buffer, 2);
  write(STDOUT_FILENO, " eating...\n", 11);

  forks[(PHILOS + id - 1) % PHILOS].dirty = false; // Dirty left fork
  forks[id].dirty = false; // Dirty right fork

  add_eater_dinner_gui(id);

  for (int i = 0; i < 100000000; i++) {
    // Delay so that gui can see who is eating
  }

}

void display_can_eat_message(int id) {
  char buffer[2];
  itoa(buffer, id);
  write(STDOUT_FILENO, buffer, 2);
  write(STDOUT_FILENO, " has both forks and can eat.\n", 29);
}

void await_request_and_give_away_fork(int id, int readfd, int writefd, char* side) {
  char buffer[2];
  char idbuff[2];
  itoa(idbuff, id);

  read(readfd, buffer, 2);

  write(STDOUT_FILENO, idbuff, 2);
  if (0 == strcmp(side, "left")) {
    write(STDOUT_FILENO, " cleaning left fork and giving it away.\n", 41);
    clean_fork(id, "left");
    give_fork(id, "left");
    remove_fork_dinner_gui(id, "left");
  } else {
    write(STDOUT_FILENO, " cleaning right fork and giving it away.\n", 41);
    clean_fork(id, "right");
    give_fork(id, "right");
    remove_eater_dinner_gui(id);
    remove_fork_dinner_gui(id, "left");
  }
  write(writefd, "yy", 2);

}

void request_fork(int id, int readfd, int writefd, char* side) {
  char buffer[2];
  char idbuff[2];
  itoa(idbuff, id);

  write(STDOUT_FILENO, idbuff, 2);
  if (0 == strcmp(side, "left")) {
    write(STDOUT_FILENO, " requesting left fork.\n", 23);
  } else {
    write(STDOUT_FILENO, " requesting right fork.\n", 24);
  }

  write(writefd, "rq", 2);
  read(readfd, buffer, 2); // should have check that you read "yy"

  write(STDOUT_FILENO, idbuff, 2);
  if (0 == strcmp(side, "left")) {
    write(STDOUT_FILENO, " received left fork.\n", 21);
    add_fork_dinner_gui(id, "left");
  } else {
    write(STDOUT_FILENO, " received right fork.\n", 22);
    add_fork_dinner_gui(id, "right");
  }
}

int get_state(int id) {
  if (has_fork(id, "left") && has_fork(id, "right")) {
    return 3;
  } else if (has_fork(id, "left") && !has_fork(id, "right")) {
    return 2;
  } else if (!has_fork(id, "left") && has_fork(id, "right")) {
    return 1;
  } else if (!has_fork(id, "left") && !has_fork(id, "right")) {
    return 0;
  }
  return -1;
}

void philo(int id, int left_read_fd, int left_write_fd, int right_read_fd, int right_write_fd) {

  char buffer[2];
  char idbuff[2];
  itoa(idbuff, id);

  int state = 0;

  display_philosopher_message(id);

  while (1) {

    state = get_state(id);

    switch (state) {
      case 3: {
        display_can_eat_message(id);
        eat(id);
        await_request_and_give_away_fork(id, right_read_fd, right_write_fd, "right");
        await_request_and_give_away_fork(id, left_read_fd, left_write_fd, "left");
        break;
      }
      case 2: {
        await_request_and_give_away_fork(id, left_read_fd, left_write_fd, "left");
        request_fork(id, right_read_fd, right_write_fd, "right");
        break;
      }
      case 1: {
        request_fork(id, left_read_fd, left_write_fd, "left");
        display_can_eat_message(id);
        eat(id);
        await_request_and_give_away_fork(id, right_read_fd, right_write_fd, "right");
        break;
      }
      case 0: {
        request_fork(id, left_read_fd, left_write_fd, "left");
        request_fork(id, right_read_fd, right_write_fd, "right");
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
