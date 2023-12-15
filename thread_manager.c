#include "thread_manager.h"
#include <string.h>

poll_t *thread_manager = NULL;

int manager_init(int max_thread, int fd_in, int fd_out) {
  if (thread_manager != NULL)
    return -1;
  thread_manager = (poll_t *)malloc(sizeof(poll_t));
  if (thread_manager == NULL)
    return -1;

  thread_manager->max_thread = (unsigned int)max_thread;
  thread_manager->fd_in = fd_in;
  thread_manager->fd_out = fd_out;
  thread_manager->threads =
      (thread_data_t *)malloc(sizeof(thread_data_t) * (size_t)max_thread);

  memset(thread_manager->threads, '\0',
         sizeof(thread_data_t) * (size_t)max_thread);

  for (size_t i = 0; i < thread_manager->max_thread; i++) {
    thread_manager->threads[i].thread_id = (intptr_t)i + 1;
  }

  if (thread_manager->threads == NULL) {
    free(thread_manager);
    thread_manager = NULL;
    return -1;
  }
  if (pthread_mutex_init(&thread_manager->time_mutex, NULL) ||
      pthread_mutex_init(&thread_manager->parse_mutex, NULL) ||
      pthread_mutex_init(&thread_manager->print_mutex, NULL)) {
    free(thread_manager->threads);
    free(thread_manager);
    thread_manager = NULL;
    return -1;
  }
  return 0;
}

/// Starts threads for given start_routine
/// @param start_routine function to be executed.
/// @return 1 if thread stopped due to a barrier, 0 otherwise.
int manager_run(void *(*start_routine)(void *)) {

  thread_manager->barred = 0;

  for (size_t i = 0; i < thread_manager->max_thread; i++) {
    thread_data_t *data = &thread_manager->threads[i];
    pthread_create(&data->thread, NULL, start_routine, (void *)data->thread_id);
  }

  for (size_t i = 0; i < thread_manager->max_thread; i++) {
    pthread_join(thread_manager->threads[i].thread, NULL);
  }

  return thread_manager->barred;
}
#include <stdio.h>

/// Attempts to get lock to read from file.
/// Exits thread if there is a barrier.
/// @param thread_id from 1 to max_thread
void manager_parse_lock(intptr_t thread_id) {
  // Lock parse mutex
  pthread_mutex_lock(&thread_manager->parse_mutex);

  // Wait time
  thread_wait(thread_id);

  // Check barrier
  if (thread_manager->barred) {
    manager_parse_unlock();
    pthread_exit((void *)1);
  }
}

int manager_parse_unlock() {
  return pthread_mutex_unlock(&thread_manager->parse_mutex);
}

void thread_wait(intptr_t thread_id) {
  unsigned int delay_ms = 0;
  wait_time_t *cur;
  wait_time_t *next;
  struct timespec delay;

  // Wait for delay and reset delay to 0
  pthread_mutex_lock(&thread_manager->time_mutex);
  next = thread_manager->threads[thread_id - 1].wait;

  while (next) {
    cur = next;
    next = cur->next;
    delay_ms += cur->delay;
    free(cur);
  }
  thread_manager->threads[thread_id - 1].wait = NULL;
  pthread_mutex_unlock(&thread_manager->time_mutex);

  if (delay_ms == 0)
    return;

  printf("Thread %lu Waiting %ums...\n", thread_id, delay_ms);

  // Allow others to parse while we sleep
  pthread_mutex_unlock(&thread_manager->parse_mutex);

  delay = (struct timespec){delay_ms / 1000, (delay_ms % 1000) * 1000000};
  nanosleep(&delay, NULL);

  pthread_mutex_lock(&thread_manager->parse_mutex);
}

void set_wait(unsigned int delay_ms, unsigned int thread_id) {
  wait_time_t *node, **next;

  if (thread_id > thread_manager->max_thread) {
    fprintf(stderr,
            "Waiting for thread_id %u is not possible with MAX_THREAD=%u",
            thread_id, thread_manager->max_thread);
    return;
  }

  pthread_mutex_lock(&thread_manager->time_mutex);
  if (thread_id == 0) {
    for (size_t i = 0; i < thread_manager->max_thread; i++) {
      node = (wait_time_t *)malloc(sizeof(wait_time_t));
      node->delay = delay_ms;
      node->next = NULL;
      next = &thread_manager->threads[i].wait;
      // Go to last ptr
      while (*next) {
        next = &(*next)->next;
      }
      // Attach
      *next = node;
    }
  } else {

    node = (wait_time_t *)malloc(sizeof(wait_time_t));
    node->delay = delay_ms;
    node->next = NULL;
    next = &thread_manager->threads[thread_id - 1].wait;
    // Go to last ptr
    while (*next) {
      next = &(*next)->next;
    }
    // Attach
    *next = node;
  }
  pthread_mutex_unlock(&thread_manager->time_mutex);
}

void manager_destroy() {
  pthread_mutex_destroy(&thread_manager->time_mutex);
  pthread_mutex_destroy(&thread_manager->parse_mutex);
  free(thread_manager->threads);
  free(thread_manager);
  thread_manager = NULL;
}
