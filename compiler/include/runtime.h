#ifndef TIGHTC_RUNTIME_H
#define TIGHTC_RUNTIME_H

#include <stdint.h>

// Goroutine/task structure
typedef struct Task {
    void (*func)(void *arg);
    void *arg;
    struct Task *next;
} Task;

// Thread pool structure
typedef struct ThreadPool {
    Task *task_queue;
    int queue_size;
    int queue_capacity;
    
    pthread_t *threads;
    int thread_count;
    
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
    pthread_cond_t queue_empty;
    
    int shutdown;
} ThreadPool;

// Queue and stack runtime structures
typedef struct Queue {
    void **items;
    int capacity;
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} Queue;

typedef struct Stack {
    void **items;
    int capacity;
    int top;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} Stack;

// Thread pool functions
ThreadPool *thread_pool_create(int thread_count);
void thread_pool_destroy(ThreadPool *pool);
void thread_pool_submit(ThreadPool *pool, void (*func)(void *), void *arg);

// Queue functions
Queue *queue_create(int capacity);
void queue_destroy(Queue *q);
void queue_push(Queue *q, void *item);
void *queue_pop(Queue *q);

// Stack functions  
Stack *stack_create(int capacity);
void stack_destroy(Stack *s);
void stack_push(Stack *s, void *item);
void *stack_pop(Stack *s);

// Global runtime state
extern ThreadPool *g_thread_pool;

// Runtime initialization
void runtime_init(void);
void runtime_shutdown(void);

#endif
