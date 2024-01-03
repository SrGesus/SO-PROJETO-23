#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/constants.h"
#include "common/io.h"
#include "common/op_code.h"
#include "server/operations.h"
#include "server/parser.h"
#include "server/queue.h"
#include "server/session.h"

int sigusr1_triggered = false;

static void sig_handler(int) { sigusr1_triggered = true; }

void *worker_thread(void *arg) {
  // printf("workerThread\n");
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
    // TRead session from producer-consumer buffer
    queue_consume(&session, session_id);

    if (write_uint(session.response_pipe, session_id)) {
      fprintf(stderr, "[ERR]: Failed to send setup response: %s\n", strerror(errno));
      break;
    }

    while (parse_operation(&session) != -1)
      ;
  }
  return (void *)0;
}

int main(int argc, char *argv[]) {
  // Check arg count
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s\n <pipe_path> [delay]\n", argv[0]);
    return 1;
  }

  // Parse access delay time
  char *endptr;
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

  // Set signal handler for sigusr1
  if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
    fprintf(stderr, "[ERR]: Failed to set signal handler\n");
    return 1;
  }

  // Initialize pipe
  int register_fifo;
  if (initialize_pipe(&register_fifo, argv[1])) {
    return 1;
  }

  // Initialize producer-consumer buffer
  if (queue_init()) return 1;

  // Create worker threads
  pthread_t threads[MAX_SESSION_COUNT];
  for (size_t i = 0; i < MAX_SESSION_COUNT; i++) {
    pthread_create(threads + i, NULL, worker_thread, (void *)i);
  }

  // HOST
  session_t session;
  while (1) {
    // Read from pipe
    if (initiate_session(&session, register_fifo)) {
      if (errno != EINTR) return 1;
    } else {
      // Write session into producer-consumer buffer
      queue_produce(&session);
    }


    if (sigusr1_triggered) {
      ems_list_every(STDOUT_FILENO);
      sigusr1_triggered = false;
    }
  }

  // Destroy producer consumer buffer
  queue_destroy();

  // Close pipe
  close(register_fifo);

  ems_terminate();
}
