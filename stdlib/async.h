#ifndef TIGHC_RUNTIME_SIMPLE_H
#define TIGHC_RUNTIME_SIMPLE_H

#include "snq.h"  // Include queue/stack definitions
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

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

// Global runtime state
ThreadPool *g_thread_pool = NULL;
static bool runtime_initialized = false;


// Forward declarations
static void ensure_runtime_initialized(void);
static void cleanup_runtime(void);

// Stub implementations for testing
ThreadPool *thread_pool_create(int thread_count) {
    (void)thread_count;
    return NULL;
}

void thread_pool_destroy(ThreadPool *pool) {
    (void)pool;
}

void thread_pool_submit(ThreadPool *pool, void (*func)(void *), void *arg) {
    (void)pool;
    ensure_runtime_initialized();  // Auto-initialize if needed
    if (func) func(arg);
}

// Lazy runtime initialization - called automatically when needed
static void ensure_runtime_initialized(void) {
    if (!runtime_initialized) {
        g_thread_pool = thread_pool_create(4);
        runtime_initialized = true;
        atexit(cleanup_runtime);  // Register automatic cleanup
    }
}

// Automatic cleanup function
static void cleanup_runtime(void) {
    if (runtime_initialized && g_thread_pool) {
        thread_pool_destroy(g_thread_pool);
        g_thread_pool = NULL;
        runtime_initialized = false;
    }
}

// Runtime initialization and cleanup (kept for backward compatibility)
void runtime_init(void) {
    ensure_runtime_initialized();
}

void runtime_shutdown(void) {
    cleanup_runtime();
}

// Convenience functions for async module (kept for backward compatibility)
void async_init(void) {
    ensure_runtime_initialized();
}

void async_shutdown(void) {
    cleanup_runtime();
}


#endif