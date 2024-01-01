#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "common/constants.h"
#include "common/io.h"
#include "common/op_code.h"
#include "server/session.h"
#include "server/parser.h"
#include "server/operations.h"

int sigusr1_triggered = false;

static void sig_handler(int) {
  sigusr1_triggered = true;
}
void * worker_thread(void *arg) {
  unsigned int session_id = (unsigned int)(intptr_t)arg;
  session_t session;
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  if (pthread_sigmask(SIG_BLOCK, &set, NULL)) {
    fprintf(stderr, "[ERR]: Failed to block signal\n");
    return (void *)1;
  }

  while (1) {
    // TODO: Read session from producer-consumer buffer

    if (write_uint(session.response_pipe, session_id)) {
      fprintf(stderr, "[ERR]: Failed to send setup response: %s\n", strerror(errno));
      break;
    }

    while (parse_operation(&session) != -1)
      ;
  }
  return (void *)0;
}

int main(int argc, char* argv[]) {
  // Check arg count
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s\n <pipe_path> [delay]\n", argv[0]);
    return 1;
  }

  // Parse access delay time
  char* endptr;
  unsigned int state_access_delay_us = STATE_ACCESS_DELAY_US;
  if (argc == 3) {
    unsigned long int delay = strtoul(argv[2], &endptr, 10);

    if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "[ERR]: Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_us = (unsigned int)delay;
  }

  // Initialize EMS
  if (ems_init(state_access_delay_us)) {
    fprintf(stderr, "[ERR]: Failed to initialize EMS\n");
    return 1;
  }

  if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
    fprintf(stderr, "[ERR]: Failed to set signal handler\n");
    return 1;
  }

  // TODO: Intialize server, create worker threads

  // Initialize pipe
  int register_fifo;
  if (initialize_pipe(&register_fifo, argv[1])) {
    return 1;
  }

  // TODO: Initialize producer-consumer buffer

  pthread_t threads[MAX_SESSION_COUNT];

  for (size_t i; i < MAX_SESSION_COUNT; i++) {
    pthread_create(threads+i, NULL, worker_thread, (void *)i /* TODO: talvez tmb producer consumer buffer*/);
  }

  session_t session;
  while (1) {
    // Read from pipe
    if (initiate_session(&session, register_fifo))
      return 1;
    // TODO: Write session into producer-consumer buffer

    if (sigusr1_triggered) {
      ems_list_every(STDOUT_FILENO);
      sigusr1_triggered = false;
    }
  }

  // TODO: Destroy producer consumer buffer

  // Close pipe
  close(register_fifo);

  ems_terminate();
}

