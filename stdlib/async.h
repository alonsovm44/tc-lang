#ifndef TIGHC_RUNTIME_SIMPLE_H
#define TIGHC_RUNTIME_SIMPLE_H

#include "snq.h"  // Include queue/stack definitions

// Thread pool structures (only for async)
typedef struct Task {
    void (*func)(void *arg);
    void *arg;
    struct Task *next;
} Task;

typedef struct ThreadPool {
    Task *task_queue;
    int queue_size;
    int queue_capacity;
    int thread_count;
    int shutdown;
} ThreadPool;

// Thread pool functions (only for async)
ThreadPool *thread_pool_create(int thread_count);
void thread_pool_destroy(ThreadPool *pool);
void thread_pool_submit(ThreadPool *pool, void (*func)(void *), void *arg);

// Global runtime state
extern ThreadPool *g_thread_pool;

// Runtime initialization
void runtime_init(void);
void runtime_shutdown(void);

#endif