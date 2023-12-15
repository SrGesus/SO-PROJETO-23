#ifndef THREAD_manager_H
#define THREAD_manager_H

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct wait_time {
  unsigned int delay;
  struct wait_time *next;
} wait_time_t;

typedef struct thread_data {
  // 1-indexed thread id
  intptr_t thread_id;
  pthread_t thread;
  wait_time_t *wait;
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
  unsigned int max_thread;
  int fd_in;
  int fd_out;
} poll_t;

extern poll_t *thread_manager;

/// Initializes the thread apparattus.
/// @param max_thread Number of threads to be run.
/// @param fd_in Input file descriptor.
/// @param fd_out Output file descriptor.
int manager_init(int max_thread, int fd_in, int fd_out);

/// Starts threads for given start_routine
/// @param start_routine function to be executed.
/// @return 1 if thread stopped due to a barrier, 0 otherwise.
int manager_run(void *(*start_routine)(void *));

/// Attempts to get lock to read from file.
/// Exits thread if there is a barrier.
/// @param thread_id from 1 to max_thread
void manager_parse_lock(intptr_t thread_id);

int manager_parse_unlock();

/// Sleeps
void thread_wait(intptr_t thread_id);

void set_wait(unsigned int delay_ms, unsigned int thread_id);

void manager_destroy();

#endif // THREAD_manager_H