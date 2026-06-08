<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: 88790d890eb81e88
timestamp: 2026-06-07T22:30:01Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# Async Runtime

This section documents the asynchronous runtime implementation provided in this code. The purpose of this runtime is to handle thread pool creation and submission, as well as queue and stack management for managing tasks.

## 1. Runtime Initialization and Cleanup

### Function: `runtime_init()`

**Purpose:** Initializes the runtime environment if not already initialized.

**Usage:** Call `runtime_init()` early in your application's startup sequence to ensure the necessary resources are available.

**Behavior:** 
- Calls `ensure_runtime_initialized()` to check for existing thread pool. If no thread pool exists, it creates one with 4 threads.
- Registers `cleanup_runtime()` as an automatic cleanup function via `atexit()`, ensuring proper shutdown even if your application exits unexpectedly.

### Function: `runtime_shutdown()`

**Purpose:** Cleans up the runtime environment.

**Usage:** Call `runtime_shutdown()` in your application's shutdown sequence to ensure all resources are properly freed and released.

**Behavior:** 
- Calls `cleanup_runtime()` to destroy any existing thread pool.
- Resets global state to indicate no active runtime is initialized, allowing for clean-up of internal state objects.

### Convenience Functions: `async_init()`, `async_shutdown()`

**Purpose:** Wrapper functions that call their respective init and shutdown functions (`runtime_init` and `runtime_shutdown`).

## 2. Thread Pool Management

### Function: `thread_pool_create(thread_count)`

**Purpose:** Creates a thread pool with the specified number of threads.

**Usage:** Use this function to create a new thread pool, which is used for managing worker tasks in asynchronous operations.

**Behavior:** 
- Returns a pointer to the newly created thread pool.
- The default number of threads (`4`) can be overridden by specifying `thread_count`.

### Function: `thread_pool_destroy(pool)`

**Purpose:** Destroys and frees resources associated with the specified thread pool.

**Usage:** Call this function when you no longer need the thread pool, which is typically after all tasks have been submitted to it.

**Behavior:** 
- Frees any resources allocated by the thread pool.
- Does nothing if `pool` is null or does not point to a valid thread pool.

### Function: `thread_pool_submit(pool, func, arg)`

**Purpose:** Submits a task for execution in the specified thread pool. The function provided as `func` will be called with the argument passed as `arg`.

**Usage:** Use this function when you want to start an asynchronous operation that can be performed concurrently.

**Behavior:** 
- Ensures runtime initialization before submitting tasks.
- If the thread pool is null or does not exist, this function returns immediately without doing anything.
- Otherwise, it delegates the task submission to `thread_pool_submit()`, which will execute the provided function with the specified argument in a new worker thread managed by the thread pool.

## 3. Queue Management

### Function: `queue_create(capacity)`

**Purpose:** Creates a queue and initializes its state based on the given capacity.

**Usage:** Use this to create an empty queue that can store elements of the size indicated by `elem_size`.

**Behavior:** 
- Initializes an empty queue with the specified capacity.
- Returns a pointer to the newly created queue object, which must be stored for future reference.

### Function: `queue_destroy(q)`

**Purpose:** Frees all resources used by the specified queue.

**Usage:** Call this function when you no longer need a queue and want to release any associated memory allocations.

**Behavior:** 
- Frees all allocated memory for the queue, including its items.
- Does nothing if `q` is null or does not point to an existing queue.

### Function: `queue_push(q, item, elem_size)`

**Purpose:** Adds a new element (item) to the tail of the queue. The size of this element must match the expected queue element size (`elem_size`).

**Usage:** Use this function when you need to enqueue an element into your queue.

**Behavior:** 
- Ensures that there is enough capacity in the queue before adding the item.
- If no free space is available, it may not add the new item immediately and will block until sufficient capacity is freed up by dequeuing elements.
- After ensuring capacity or successfully allocating space for the new element, it moves the tail pointer forward to accommodate the added item.

### Function: `queue_pop(q)`

**Purpose:** Retrieves an element from the head of the queue. Returns null if no items are available.

**Usage:** Use this function when you need to dequeue an element from your queue and process its contents.

**Behavior:** 
- Retrieves the oldest element from the front (head) of the queue.
- If the queue is empty or has insufficient capacity, it returns `NULL` immediately without blocking.
- After popping an item, adjusts the tail pointer if necessary to maintain correct FIFO order.

### Function: `queue_peek(q)`

**Purpose:** Retrieves the top element from the queue (the oldest). Returns null if no items are available.

**Usage:** Use this function when you need a reference to the frontmost item in your queue without removing it.

**Behavior:** 
- Returns the address of the front-most element (head) as stored within the queue.
- If there are no elements, `NULL` is returned immediately without modifying any internal pointers or allocating space for additional items.

## 4. Stack Management

### Function: `stack_create(capacity)`

**Purpose:** Creates a stack and initializes its state based on the given capacity.

**Usage:** Use this to create an empty stack that can store elements of the size indicated by `elem_size`.

**Behavior:** 
- Initializes an empty stack with the specified capacity.
- Returns a pointer to the newly created stack object, which must be stored for future reference.

### Function: `stack_destroy(s)`

**Purpose:** Frees all resources used by the specified stack.

**Usage:** Call this function when you no longer need a stack and want to release any associated memory allocations.

**Behavior:** 
- Frees all allocated memory for the stack, including its items.
- Does nothing if `s` is null or does not point to an existing stack.

### Function: `stack_push(s, item, elem_size)`

**Purpose:** Adds a new element (item) to the top of the stack. The size of this element must match the expected stack element size (`elem_size`).

**Usage:** Use this function when you need to push an element onto your stack.

**Behavior:** 
- Ensures that there is enough capacity in the stack before adding the item.
- If no free space is available, it may not add the new item immediately and will block until sufficient capacity is freed up by popping elements from the top of the stack.
- After ensuring capacity or successfully allocating space for the new element, it moves the top pointer down to accommodate the added item.

### Function: `stack_pop(s)`

**Purpose:** Removes and retrieves an element from the top of the stack. Returns null if no items are available.

**Usage:** Use this function when you need to remove and retrieve the topmost item in your stack for further processing.

**Behavior:** 
- Retrieves the most recent (top-most) element from the stack.
- If the stack is empty, it returns `NULL` immediately without modifying any internal pointers or allocating space for additional items.

### Function: `stack_peek(s)`

**Purpose:** Retrieves a reference to the topmost element in your stack. Returns null if no elements are available.

**Usage:** Use this function when you need to inspect the current top item of your stack without removing it.

**Behavior:** 
- Returns the address of the top-most element (head) as stored within the stack.
- If there are no elements, `NULL` is returned immediately without modifying any internal pointers or allocating space for additional items.