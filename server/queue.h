#ifndef SERVER_QUEUE_H
#define SERVER_QUEUE_H
#include <pthread.h>
#include "session.h"
#include "common/constants.h"

typedef struct {
  session_t buffer[QUEUE_BUFFER_SIZE];
  pthread_mutex_t mutex;
  pthread_cond_t full;
  pthread_cond_t empty;
  int session_count;
  int host;
  int worker;
} queue_t;

/// @brief Initializes global pro
/// @return 
int queue_init();

void queue_destroy();

void queue_produce(session_t *session);

void queue_consume(session_t *session, unsigned int session_id);

#endif  // SERVER_QUEUE_H
