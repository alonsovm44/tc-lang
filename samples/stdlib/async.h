#ifndef STDLIB_ASYNC_H
#define STDLIB_ASYNC_H

#include <stdint.h>

// Forward declarations for runtime types
typedef struct ThreadPool ThreadPool;
typedef struct Queue Queue;
typedef struct Stack Stack;

// Runtime initialization and cleanup
void runtime_init(void);
void runtime_shutdown(void);

// Thread pool operations
ThreadPool *thread_pool_create(int thread_count);
void thread_pool_destroy(ThreadPool *pool);
void thread_pool_submit(ThreadPool *pool, void (*func)(void *), void *arg);

// Queue operations
Queue *queue_create(int capacity);
void queue_destroy(Queue *q);
void queue_push(Queue *q, void *item);
void *queue_pop(Queue *q);

// Stack operations
Stack *stack_create(int capacity);
void stack_destroy(Stack *s);
void stack_push(Stack *s, void *item);
void *stack_pop(Stack *s);

// Global thread pool instance
extern ThreadPool *g_thread_pool;

#endif
