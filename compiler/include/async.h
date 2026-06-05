#ifndef TIGHTC_RUNTIME_SIMPLE_H
#define TIGHTC_RUNTIME_SIMPLE_H

#include <stdint.h>

// Simplified runtime structures for testing
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

typedef struct Queue {
    void *items;
    int capacity;
    int head;
    int tail;
    int count;
    int elem_size;
} Queue;

typedef struct Stack {
    void *items;
    int capacity;
    int top;
    int elem_size;
} Stack;

// Thread pool functions
ThreadPool *thread_pool_create(int thread_count);
void thread_pool_destroy(ThreadPool *pool);
void thread_pool_submit(ThreadPool *pool, void (*func)(void *), void *arg);

// Queue functions
Queue queue_create(int capacity);
void queue_destroy(Queue *q);
void queue_push(Queue *q, void *item, int elem_size);
void *queue_pop(Queue *q);
void *queue_peek(Queue *q);

// Stack functions  
Stack stack_create(int capacity);
void stack_destroy(Stack *s);
void stack_push(Stack *s, void *item, int elem_size);
void *stack_pop(Stack *s);
void *stack_peek(Stack *s);

// Global runtime state
extern ThreadPool *g_thread_pool;

// Runtime initialization
void runtime_init(void);
void runtime_shutdown(void);

#endif
