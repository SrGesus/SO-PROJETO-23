#ifndef THREAD_manager_H
#define THREAD_manager_H

#include <pthread.h>
#include <stdlib.h>

typedef struct thread_data {
  // 1-indexed thread id
  int thread_id;
  pthread_t thread;
  unsigned int wait;
  // int status;
} thread_data_t;

typedef struct manager {
  // Mutex for writing in threads.time
  pthread_mutex_t time_mutex;
  // Mutex for reading from fd_in
  pthread_mutex_t parse_mutex;
  // Mutex for writing in fd_out
  pthread_mutex_t print_mutex;
  // If true exits all threads
  int barred;
  // Array for threads
  thread_data_t *threads;
  int max_thread;
  int fd_in;
  int fd_out;
} poll_t;

extern poll_t *thread_manager;

int manager_init(int max_thread, int fd_in, int fd_out);

/// Starts threads for given start_routine
/// @param start_routine function to be executed.
/// @return 1 if thread stopped due to a barrier, 0 otherwise.
int manager_run(void *(*start_routine)(void *));

/// Attempts to get lock to read from file.
/// Exits thread if there is a barrier.
/// @param thread_id from 1 to max_thread
void manager_parse_lock(size_t thread_id);

int manager_parse_unlock();

void wait_time(size_t thread_id);

void set_wait(unsigned int delay_ms, size_t thread_id);

void manager_destroy();

#endif // THREAD_manager_H