#ifndef THREAD_WAITER_H
#define THREAD_WAITER_H

#include <pthread.h>
#include <stdlib.h> 


typedef struct waiter {
    pthread_rwlock_t rwlock;
    // Delay times in ms
    unsigned long * times;
} * Waiter;

static Waiter thread_waiter;

int waiter_init(size_t max_thread) {
    thread_waiter = (struct waiter *)malloc(sizeof(struct waiter));
    if (!thread_waiter)
        return -1;
    thread_waiter->times = (unsigned long *)malloc(sizeof(unsigned long)*max_thread);
    if (!(thread_waiter->times))
        return -1;
    if (pthread_rwlock_init(&thread_waiter->rwlock, {0})) {
        return -1;
    }
    return 0;
}

void wait_time(size_t thread_id) {
    pthread_rwlock_rdlock(&thread_waiter->rwlock);
    
    pthread_rwlock_unlock(&thread_waiter->rwlock);
}

inline void waiter_destroy() {
    pthread_rwlock_destroy(&thread_waiter->rwlock);
    free(thread_waiter->times);
    free(thread_waiter);
}

#endif // THREAD_WAITER_H