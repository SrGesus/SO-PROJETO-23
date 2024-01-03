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

#include "../common/constants.h"
#include "common/io.h"
#include "common/op_code.h"
#include "server/session.h"
#include "server/parser.h"
#include "server/operations.h"
#include "server/queue.h"

int sigusr1_triggered = false;
session_t buffer[MAX_SESSION_COUNT];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER ,
     not_full = PTHREAD_COND_INITIALIZER;
int session_count = 0, host = 0, worker = 0;

int buffer_init() {
    for (size_t i = 0; i < MAX_SESSION_COUNT; i++) {
        session_t *session = malloc(sizeof(session_t));
        if (!session) {
            fprintf(stderr, "[ERR]: Failed to alloc memory for sessions\n");
            return 1;
        }
        buffer[i] = *session;
    }
    return 0;
}

void destroy_buffer(){
    for (size_t i = 0; i < MAX_SESSION_COUNT; i++){
        if (&(buffer[i]) != NULL)
            free(&(buffer[i]));
    }
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&full);
    pthread_cond_destroy(&not_full);
}
static void sig_handler(int) {
  sigusr1_triggered = true;
}

void * worker_thread(void *arg) {
  //printf("workerThread\n");
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
    if (DEBUG_THREADS) printf("[THREAD] number %u\n",session_id);
    pthread_mutex_lock(&mutex);
    //printf("session_count: %d\n",session_count);
    while (session_count == 0){
      if (DEBUG_THREADS) printf("[THREAD] number %u is waiting...\n",session_id);
      pthread_cond_wait(&not_full,&mutex);
    }
    session = buffer[worker];
    worker++; if (worker == MAX_SESSION_COUNT) worker = 0;
    session_count--;
    pthread_cond_signal(&full);
    pthread_mutex_unlock(&mutex);
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
  if (buffer_init()) return 1;

  pthread_t threads[MAX_SESSION_COUNT];

  for (size_t i = 0; i < MAX_SESSION_COUNT; i++) {
    pthread_create(threads+i, NULL, worker_thread, (void *)i /* TODO: talvez tmb producer consumer buffer*/);
  }

  //HOST
  session_t session;
  while (1) {
    // Read from pipe
    if (initiate_session(&session, register_fifo))
      return 1;
    pthread_mutex_lock(&mutex);
    while (session_count == MAX_SESSION_COUNT) pthread_cond_wait(&full,&mutex);
    buffer[host] = session;
    host++; if (host == MAX_SESSION_COUNT) host = 0;
    session_count++;
    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&mutex);

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

