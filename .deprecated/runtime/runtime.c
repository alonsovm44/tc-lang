// Simplified runtime for testing - stub implementations
#include "async.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>

// Global runtime state
ThreadPool *g_thread_pool = NULL;
static bool runtime_initialized = false;

// Error handling state
static jmp_buf *g_current_try_buf = NULL;
static char g_current_error_name[256] = {0};

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

// Error handling functions
void tc_set_try_buf(jmp_buf *buf) {
    g_current_try_buf = buf;
}

jmp_buf *tc_get_try_buf(void) {
    return g_current_try_buf;
}

void tc_throw(const char *error_name) {
    if (g_current_try_buf) {
        strncpy(g_current_error_name, error_name, sizeof(g_current_error_name) - 1);
        g_current_error_name[sizeof(g_current_error_name) - 1] = '\0';
        longjmp(*g_current_try_buf, 1);
    } else {
        fprintf(stderr, "Uncaught error: %s\n", error_name);
        exit(1);
    }
}

const char *tc_error_name(void) {
    return g_current_error_name;
}
