#ifndef THREAD_WAITER_H
#define THREAD_WAITER_H

#include <pthread.h>
#include <stdlib.h> 


typedef struct waiter {
    pthread_rwlock_t rwlock;
    // Delay times in ms
    unsigned int * times;
} * Waiter;

static Waiter thread_waiter;

int waiter_init(size_t max_thread) {
    thread_waiter = (struct waiter *)malloc(sizeof(struct waiter));
    if (!thread_waiter)
        return -1;
    thread_waiter->times = (unsigned int *)malloc(sizeof(unsigned int)*max_thread);
    if (!(thread_waiter->times))
        return -1;
    if (pthread_rwlock_init(&thread_waiter->rwlock, {0})) {
        return -1;
    }
    return 0;
}

void wait_time(size_t thread_id) {
    unsigned int delay_ms;
    struct timespec delay;

    pthread_rwlock_rdlock(&thread_waiter->rwlock);
    delay_ms = thread_waiter->times[thread_id-1];
    pthread_rwlock_unlock(&thread_waiter->rwlock);

    if (!delay_ms)
        return;
    
    // Wait for delay and reset delay to 0
    pthread_rwlock_wrlock(&thread_waiter->rwlock);
    thread_waiter->times[thread_id-1] = 0;
    pthread_rwlock_unlock(&thread_waiter->rwlock);
    delay = (struct timespec){delay_ms / 1000, (delay_ms % 1000) * 1000000};
    nanosleep(&delay, NULL);
}

inline void waiter_destroy() {
    pthread_rwlock_destroy(&thread_waiter->rwlock);
    free(thread_waiter->times);
    free(thread_waiter);
}

#endif // THREAD_WAITER_H