<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: 7c7425a88d71f6cc
timestamp: 2026-06-07T22:08:27Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# TIGHTC Runtime Documentation

## Overview

This module provides the necessary structures, functions, and global variables for managing threads, tasks, queues, and stacks in a thread-safe manner. The purpose of this library is to facilitate efficient concurrent execution within an application.

## Structures and Typedefs

### Task Structure

```markdown
typedef struct Task {
    void (*func)(void *arg);
    void *arg;
    struct Task *next;
} Task;

```

- `Task`: This structure represents a task or goroutine. It contains the function pointer that is executed, an argument (`arg`) passed to the function, and a reference to the next task in the list.

### ThreadPool Structure

```markdown
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

```

- `ThreadPool`: This structure encapsulates all the necessary components for managing a pool of threads. It includes:
  - A pointer to a list (`task_queue`) of tasks.
  - The size and capacity of the task queue.
  - An array of thread handles (`threads`), each representing an active thread in the pool.
  - Mutexes (`queue_mutex`, `queue_cond`, `queue_empty`) for controlling access to shared resources within the queue.
  - A flag (`shutdown`) indicating if the pool is shutting down.

### Queue Structure

```markdown
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

```

- `Queue`: This structure represents a data structure that can be used as both an array-based queue and stack. It includes:
  - An array of pointers (`items`) to elements.
  - The current size (`capacity`, `head`, `tail`, `count`), which is useful for operations like pushing, popping, and peeking.
  - A mutex (`mutex`) for thread safety.
  - Conditional variables (`not_empty`, `not_full`) used in synchronization.

### Stack Structure

```markdown
typedef struct Stack {
    void **items;
    int capacity;
    int top;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} Stack;

```

- `Stack`: This structure represents a last-in-first-out (LIFO) data structure. It includes:
  - An array of pointers (`items`) to elements.
  - The current size, capacity, and top index (`capacity`, `head`, `top`), which are useful for operations like pushing, popping, and peeking.
  - A mutex (`mutex`) for thread safety.
  - Conditional variables (`not_empty`, `not_full`) used in synchronization.

## Functions

### Thread Pool Functions

```markdown
ThreadPool *thread_pool_create(int thread_count);
void thread_pool_destroy(ThreadPool *pool);
void thread_pool_submit(ThreadPool *pool, void (*func)(void *), void *arg);
```

- `ThreadPool *thread_pool_create(int thread_count)`: Creates a new ThreadPool with the specified number of threads. Returns a pointer to the newly created pool.
- `void thread_pool_destroy(ThreadPool *pool)`: Destroys a ThreadPool and waits for all its tasks to complete, then cleans up resources.
- `void thread_pool_submit(ThreadPool *pool, void (*func)(void *), void *arg)`: Submits a new task to be executed by one of the threads in the pool. The provided function will be executed with the given argument.

### Queue Functions

```markdown
Queue queue_create(int capacity);
void queue_destroy(Queue *q);
void queue_push(Queue *q, void *item, int elem_size);
void *queue_pop(Queue *q);
void *queue_peek(Queue *q);
```

- `Queue queue_create(int capacity)`: Creates a new Queue with the specified capacity.
- `void queue_destroy(Queue *q)`: Destroys the provided Queue and frees its allocated memory.
- `void queue_push(Queue *q, void *item, int elem_size)`: Pushes an item into the front of the Queue. The size argument is used to determine how many bytes are pushed, which may differ from the actual element size.
- `void *queue_pop(Queue *q)`: Pops and returns the first element in the Queue. If called when the queue is empty, it blocks until an item becomes available.
- `void *queue_peek(Queue *q)`: Returns a pointer to the first element in the Queue without removing it. If called when the queue is empty, it blocks until an item becomes available.

### Stack Functions

```markdown
Stack stack_create(int capacity);
void stack_destroy(Stack *s);
void stack_push(Stack *s, void *item, int elem_size);
void *stack_pop(Stack *s);
void *stack_peek(Stack *s);
```

- `Stack stack_create(int capacity)`: Creates a new Stack with the specified capacity.
- `void stack_destroy(Stack *s)`: Destroys the provided Stack and frees its allocated memory.
- `void stack_push(Stack *s, void *item, int elem_size)`: Pushes an item onto the top of the Stack. The size argument is used to determine how many bytes are pushed, which may differ from the actual element size.
- `void *stack_pop(Stack *s)`: Pops and returns the top element in the Stack. If called when the stack is empty, it blocks until an item becomes available.
- `void *stack_peek(Stack *s)`: Returns a pointer to the top element in the Stack without removing it. If called when the stack is empty, it blocks until an item becomes available.

## Global State

```markdown
extern ThreadPool *g_thread_pool;

```

The global variable `g_thread_pool` points to the currently active thread pool. It can be used for interacting with thread pools that have been created elsewhere in the application.

## Runtime Initialization and Shutdown

```markdown
void runtime_init(void);
void runtime_shutdown(void);

```

- `void runtime_init(void)`: This function initializes any necessary global state required for thread safety, including initializing mutexes and condition variables. It should be called early in the startup process.
- `void runtime_shutdown(void)`: This function ensures all threads complete their current tasks before shutting down. After this call, no more new tasks can be submitted to the pool, and the pool is ready to be destroyed.

## Security Considerations

When working with thread-safe data structures like queues and stacks, care must be taken to synchronize access through mutexes and condition variables. Failure to do so can lead to race conditions or deadlocks. Ensure that all shared resources are properly protected by these mechanisms.

For example:
- Always lock the queue's mutex before performing operations on it.
- Use `pthread_cond_t` signals (e.g., `pthread_cond_signal`) appropriately to wake up waiting threads, but never directly modify any shared state without acquiring appropriate locks first.