// Simplified runtime for testing - stub implementations
#include "async.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// Global runtime state
ThreadPool *g_thread_pool = NULL;
static bool runtime_initialized = false;

// Forward declarations
static void ensure_runtime_initialized(void);
static void cleanup_runtime(void);

// Stub implementations for testing
ThreadPool *thread_pool_create(int thread_count) {
    return NULL;
}

void thread_pool_destroy(ThreadPool *pool) {
}

void thread_pool_submit(ThreadPool *pool, void (*func)(void *), void *arg) {
    ensure_runtime_initialized();  // Auto-initialize if needed
    if (func) func(arg);
}

Queue *queue_create(int capacity) {
    return NULL;
}

void queue_destroy(Queue *q) {
}

void queue_push(Queue *q, void *item) {
}

void *queue_pop(Queue *q) {
    return NULL;
}

Stack *stack_create(int capacity) {
    return NULL;
}

void stack_destroy(Stack *s) {
}

void stack_push(Stack *s, void *item) {
}

void *stack_pop(Stack *s) {
    return NULL;
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

