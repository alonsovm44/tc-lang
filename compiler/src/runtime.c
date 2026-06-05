// Simplified runtime for testing - stub implementations
#include "async.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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

static bool queue_ensure_capacity(Queue *q, int elem_size) {
    if (!q) return false;
    if (elem_size <= 0) return false;

    if (!q->items) {
        q->elem_size = elem_size;
        q->items = calloc((size_t)q->capacity, (size_t)elem_size);
        return q->items != NULL;
    }
    if (q->elem_size != elem_size) {
        return false;
    }
    if (q->count < q->capacity) {
        return true;
    }

    int new_capacity = q->capacity * 2;
    void *new_items = calloc((size_t)new_capacity, (size_t)elem_size);
    if (!new_items) return false;

    for (int i = 0; i < q->count; i++) {
        char *src = (char *)q->items + (size_t)(((q->head + i) % q->capacity) * q->elem_size);
        char *dst = (char *)new_items + (size_t)(i * q->elem_size);
        memcpy(dst, src, (size_t)q->elem_size);
    }

    free(q->items);
    q->items = new_items;
    q->capacity = new_capacity;
    q->head = 0;
    q->tail = q->count;
    return true;
}

Queue queue_create(int capacity) {
    Queue q = {0};
    if (capacity <= 0) capacity = 8;
    q.capacity = capacity;
    q.items = NULL;
    q.head = 0;
    q.tail = 0;
    q.count = 0;
    q.elem_size = 0;
    return q;
}

void queue_destroy(Queue *q) {
    if (!q) return;
    free(q->items);
}

void queue_push(Queue *q, void *item, int elem_size) {
    if (!q || !item || elem_size <= 0) return;
    if (!queue_ensure_capacity(q, elem_size)) return;
    char *dst = (char *)q->items + (size_t)(q->tail * q->elem_size);
    memcpy(dst, item, (size_t)q->elem_size);
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
}

void *queue_pop(Queue *q) {
    if (!q || q->count == 0 || !q->items) return NULL;
    char *src = (char *)q->items + (size_t)(q->head * q->elem_size);
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    return src;
}

void *queue_peek(Queue *q) {
    if (!q || q->count == 0 || !q->items) return NULL;
    return (char *)q->items + (size_t)(q->head * q->elem_size);
}

static bool stack_ensure_capacity(Stack *s, int elem_size) {
    if (!s) return false;
    if (elem_size <= 0) return false;

    if (!s->items) {
        s->elem_size = elem_size;
        s->items = calloc((size_t)s->capacity, (size_t)elem_size);
        return s->items != NULL;
    }
    if (s->elem_size != elem_size) {
        return false;
    }
    if (s->top < s->capacity) {
        return true;
    }

    int new_capacity = s->capacity * 2;
    void *new_items = calloc((size_t)new_capacity, (size_t)elem_size);
    if (!new_items) return false;

    memcpy(new_items, s->items, (size_t)s->capacity * (size_t)s->elem_size);
    free(s->items);
    s->items = new_items;
    s->capacity = new_capacity;
    return true;
}

Stack stack_create(int capacity) {
    Stack s = {0};
    if (capacity <= 0) capacity = 8;
    s.capacity = capacity;
    s.items = NULL;
    s.top = 0;
    s.elem_size = 0;
    return s;
}

void stack_destroy(Stack *s) {
    if (!s) return;
    free(s->items);
}

void stack_push(Stack *s, void *item, int elem_size) {
    if (!s || !item || elem_size <= 0) return;
    if (!stack_ensure_capacity(s, elem_size)) return;
    char *dst = (char *)s->items + (size_t)(s->top * s->elem_size);
    memcpy(dst, item, (size_t)s->elem_size);
    s->top++;
}

void *stack_pop(Stack *s) {
    if (!s || s->top == 0 || !s->items) return NULL;
    s->top--;
    return (char *)s->items + (size_t)(s->top * s->elem_size);
}

void *stack_peek(Stack *s) {
    if (!s || s->top == 0 || !s->items) return NULL;
    return (char *)s->items + (size_t)((s->top - 1) * s->elem_size);
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

