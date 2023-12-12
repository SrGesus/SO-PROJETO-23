#ifndef thread_pool_H
#define thread_pool_H

#include <pthread.h>
#include <stdlib.h> 

typedef struct thread_args {
    // 1-indexed thread id
    size_t thread_id;
    pthread_t thread;
} thread_args_t;

typedef struct pool {
    pthread_rwlock_t time_lock;
    pthread_rwlock_t parse_lock;
    int barred;
    // Delay times in ms
    unsigned int * times;
    thread_args_t * threads;
} poll_t;

static poll_t * thread_pool;

int pool_init(size_t max_thread) {
    if (thread_pool != nullptr)
        return -1;
    thread_pool = (poll_t *)malloc(sizeof(poll_t));
    if (thread_pool  == nullptr)
        return -1;
    thread_pool->times = (unsigned int *)malloc(sizeof(unsigned int)*max_thread);
    if (thread_pool->times == nullptr)
        return -1;
    thread_pool->threads = (thread_args_t *)malloc(sizeof(thread_args_t)*max_thread);
    if (thread_pool->threads  == nullptr)
        return -1;
    if (pthread_rwlock_init(&thread_pool->time_lock, nullptr)) {
        return -1;
    }
    if (pthread_rwlock_init(&thread_pool->parse_lock, nullptr)) {
        return -1;
    }
    return 0;
}

// @param thread_id from 1 to max_thread
/// @return 1 if a barrier was active, 0 otherwise
int pool_parse_lock(size_t thread_id) {
    // Wait time
    wait_time(thread_id);


    // Check barrier
    if (thread_pool->barred)

}

void wait_time(size_t thread_id) {
    unsigned int delay_ms;
    struct timespec delay;

    pthread_rwlock_rdlock(&thread_pool->time_lock);
    delay_ms = thread_pool->times[thread_id-1];
    pthread_rwlock_unlock(&thread_pool->time_lock);

    if (delay_ms == 0)
        return;
    
    // Wait for delay and reset delay to 0
    pthread_rwlock_wrlock(&thread_pool->time_lock);
    thread_pool->times[thread_id-1] = 0;
    pthread_rwlock_unlock(&thread_pool->time_lock);
    delay = (struct timespec){delay_ms / 1000, (delay_ms % 1000) * 1000000};
    nanosleep(&delay, NULL);
}

inline void pool_destroy() {
    pthread_rwlock_destroy(&thread_pool->time_lock);
    free(thread_pool->times);
    free(thread_pool->threads);
    free(thread_pool);
}

#endif // thread_pool_H