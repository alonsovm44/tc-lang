<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: ad5efce36d5800a1
timestamp: 2026-06-07T21:59:19Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# TIGHTC_RUNTIME_SIMPLE_H

## Overview
This header file provides a simplified set of structures and functions for managing tasks, queues, stacks, and threads within the TIGHTC framework. The goal is to provide low-level abstractions that can be used in testing and debugging environments.

## Structs Definitions

### Task
A structure representing a task with associated function pointer and argument.

```c
typedef struct Task {
    void (*func)(void *arg); // Function pointer for the task's execution.
    void *arg;                // Argument to be passed to the function.
    struct Task *next;        // Pointer to the next task in queue.
} Task;
```

### ThreadPool
A structure representing a thread pool which manages tasks.

```c
typedef struct ThreadPool {
    Task *task_queue;       // Queue of tasks that need execution.
    int queue_size;          // Current number of items in task queue.
    int queue_capacity;      // Maximum capacity of the task queue.
    int thread_count;        // Number of threads managed by the pool.
    int shutdown;            // Flag indicating whether the thread pool is shutting down.
} ThreadPool;
```

### Queue
A structure representing a generic, fixed-size queue.

```c
typedef struct Queue {
    void *items;      // Pointer to allocated storage for items.
    int capacity;     // Maximum number of elements that can be stored in this queue.
    int head;         // Index of the first element of an array holding data.
    int tail;         // Index of next available space in array.
    int count;        // Number of elements currently contained within this queue.
    int elem_size;    // Size, in bytes, of each element contained by a Queue.
} Queue;
```

### Stack
A structure representing a generic fixed-size stack.

```c
typedef struct Stack {
    void *items;   // Pointer to allocated storage for items.
    int capacity;  // Maximum number of elements that can be stored in this stack.
    int top;       // Index of the most recently added element.
    int elem_size; // Size, in bytes, of each element contained by a Stack.
} Stack;
```

## Function Definitions

### thread_pool_create
Creates and initializes a new thread pool.

```c
ThreadPool *thread_pool_create(int thread_count);
```
- **Parameters:** `thread_count` - Number of threads to create for the pool.
- **Return Value:** A pointer to the newly created thread pool structure, or NULL if an error occurs.

### thread_pool_destroy
Destroys a given thread pool and releases its resources.

```c
void thread_pool_destroy(ThreadPool *pool);
```
- **Parameters:** `pool` - Pointer to the ThreadPool structure to be destroyed.
- **Return Value:** None

### thread_pool_submit
Submits a task to the thread pool for execution. The task is expected to be non-blocking.

```c
void thread_pool_submit(ThreadPool *pool, void (*func)(void *), void *arg);
```
- **Parameters:** `pool` - Pointer to the ThreadPool structure.
- `func` - Function pointer of type `void (*)(void *)` that will be called by the pool. 
- `arg` - Argument for the function passed in.

### queue_create
Creates a new, empty queue with a specified capacity.

```c
Queue *queue_create(int capacity);
```
- **Parameters:** `capacity` - The number of items that can be stored in this queue.
- **Return Value:** A pointer to the newly created Queue structure or NULL if an error occurs.

### queue_destroy
Destroys and frees resources associated with a given queue.

```c
void queue_destroy(Queue *q);
```
- **Parameters:** `q` - Pointer to the Queue structure to be destroyed.
- **Return Value:** None

### queue_push
Adds an element to the rear of the queue. The new item is allocated from memory managed by the queue, so it must not be freed.

```c
void queue_push(Queue *q, void *item, int elem_size);
```
- **Parameters:** `q` - Pointer to the Queue structure.
- `item` - The data for this element. Must be a pointer of type `elem_size`.
- `elem_size` - Size in bytes of each item stored in the queue.

### queue_pop
Removes and returns an element from the front of the queue. A NULL value is returned on failure.

```c
void *queue_pop(Queue *q);
```
- **Parameters:** `q` - Pointer to the Queue structure.
- **Return Value:** The data for the first element in the queue or NULL if it is full.

### queue_peek
Retrieves an element from the front of the queue without removing it. A NULL value is returned on failure.

```c
void *queue_peek(Queue *q);
```
- **Parameters:** `q` - Pointer to the Queue structure.
- **Return Value:** The data for the first element in the queue or NULL if it is empty.

### stack_create
Creates a new, empty stack with a specified capacity.

```c
Stack *stack_create(int capacity);
```
- **Parameters:** `capacity` - Maximum number of elements that can be stored in this stack.
- **Return Value:** A pointer to the newly created Stack structure or NULL if an error occurs.

### stack_destroy
Destroys and frees resources associated with a given stack.

```c
void stack_destroy(Stack *s);
```
- **Parameters:** `s` - Pointer to the Stack structure to be destroyed.
- **Return Value:** None

### stack_push
Adds an element to the top of the stack. The new item is allocated from memory managed by the stack, so it must not be freed.

```c
void stack_push(Stack *s, void *item, int elem_size);
```
- **Parameters:** `s` - Pointer to the Stack structure.
- `item` - Data for this element. Must be a pointer of type `elem_size`.
- `elem_size` - Size in bytes of each item stored in the stack.

### stack_pop
Removes and returns an element from the top of the stack.

```c
void *stack_pop(Stack *s);
```
- **Parameters:** `s` - Pointer to the Stack structure.
- **Return Value:** The data for the last element in the stack or NULL if it is empty.

### stack_peek
Retrieves an element from the top of the stack without removing it. A NULL value is returned on failure.

```c
void *stack_peek(Stack *s);
```
- **Parameters:** `s` - Pointer to the Stack structure.
- **Return Value:** The data for the last element in the stack or NULL if it is empty.

### runtime_init
Initializes the global runtime state. This function should be called before any other runtime functions are used.

```c
void runtime_init(void);
```
- **Parameters:** None
- **Return Value:** None

### runtime_shutdown
Shuts down the global runtime state, destroying all thread pools and queues that were created during initialization.

```c
void runtime_shutdown(void);
```
- **Parameters:** None
- **Return Value:** None

## External Variable

### g_thread_pool
A pointer to a global ThreadPool structure. This variable is used by internal functions to manage the main application's thread pool.

```c
extern ThreadPool *g_thread_pool;
```