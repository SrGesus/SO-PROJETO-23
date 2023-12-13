#ifndef THREAD_manager_H
#define THREAD_manager_H

#include <pthread.h>
#include <stdlib.h>

typedef struct thread_data {
  // 1-indexed thread id
  size_t thread_id;
  pthread_t thread;
  unsigned int wait;
  // int status;
} thread_data_t;

typedef struct manager {
  pthread_mutex_t time_mutex;
  pthread_mutex_t parse_mutex;
  pthread_mutex_t print_mutex;
  // If true exits all threads
  int barred;
  // Array for threads
  thread_data_t *threads;
  size_t max_thread;
  int fd_in;
  int fd_out;
} poll_t;

static poll_t *thread_manager = NULL;

int manager_init(size_t max_thread, int fd_in, int fd_out) {
  if (thread_manager != NULL)
    return -1;
  thread_manager = (poll_t *)malloc(sizeof(poll_t));
  if (thread_manager == NULL)
    return -1;
  thread_manager->threads =
      (thread_data_t *)malloc(sizeof(thread_data_t) * max_thread);
  if (thread_manager->threads == NULL) {
    free(thread_manager);
    thread_manager = NULL;
    return -1;
  }
  if (pthread_mutex_init(&thread_manager->time_mutex, NULL) ||
      pthread_mutex_init(&thread_manager->parse_mutex, NULL)) {
    free(thread_manager->threads);
    free(thread_manager);
    thread_manager = NULL;
    return -1;
  }
  thread_manager->max_thread = max_thread;
  thread_manager->fd_in = fd_in;
  thread_manager->fd_out = fd_out;
  for (int i = 0; i < thread_manager->max_thread; i++)
    thread_manager->threads[i].thread_id = i + 1;
  return 0;
}

/// Starts threads for given start_routine
/// @param start_routine function to be executed.
/// @return 1 if thread stopped due to a barrier, 0 otherwise.
int manager_run(void *(*start_routine)(void *)) {

  thread_manager->barred = 0;

  for (int i = 0; i < thread_manager->max_thread; i++) {
    thread_data_t *data = &thread_manager->threads[i];
    pthread_create(&data->thread, NULL, start_routine, &data->thread_id);
  }

  for (int i = 0; i < thread_manager->max_thread; i++) {
    pthread_join(thread_manager->threads[i].thread, NULL);
  }

  return thread_manager->barred;
}

/// Attempts to get lock to read from file.
/// Exits thread if there is a barrier.
/// @param thread_id from 1 to max_thread
void manager_parse_lock(size_t thread_id) {
  // Wait time
  wait_time(thread_id);

  // Lock parse mutex
  pthread_mutex_lock(&thread_manager->parse_mutex);

  // Check barrier
  if (thread_manager->barred) {
    manager_parse_unlock();
    pthread_exit((void *)1);
  }
}

int manager_parse_unlock() {
  pthread_mutex_unlock(&thread_manager->parse_mutex);
}

void wait_time(size_t thread_id) {
  unsigned int delay_ms;
  struct timespec delay;

  // Wait for delay and reset delay to 0
  pthread_mutex_lock(&thread_manager->time_mutex);
  delay_ms = thread_manager->threads[thread_id - 1].wait;
  thread_manager->threads[thread_id - 1].wait = 0;
  pthread_mutex_unlock(&thread_manager->time_mutex);

  if (delay_ms == 0)
    return;

  delay = (struct timespec){delay_ms / 1000, (delay_ms % 1000) * 1000000};
  nanosleep(&delay, NULL);
}

void set_wait(unsigned int delay_ms, size_t thread_id) {
  pthread_mutex_lock(&thread_manager->time_mutex);
  if (thread_id == 0)
    for (int i = 0; i <= thread_manager->max_thread; i++)
      thread_manager->threads[i].wait = delay_ms;
  else
    thread_manager->threads[thread_id - 1].wait = delay_ms;
  pthread_mutex_unlock(&thread_manager->time_mutex);
}

inline void manager_destroy() {
  pthread_mutex_destroy(&thread_manager->time_mutex);
  pthread_mutex_destroy(&thread_manager->parse_mutex);
  free(thread_manager->threads);
  free(thread_manager);
  thread_manager = NULL;
}

#endif // THREAD_manager_H