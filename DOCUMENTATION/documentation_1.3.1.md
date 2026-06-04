# Tig 1.3.1 Specification: First-Class Queues, Stacks, and Slices

## Overview

Tig 1.3.1 elevates queues and stacks from runtime constructs to first-class language types with automatic initialization, zero-copy slicing, and pointer semantics. This release introduces:

1. **Automatic Initialization** — Queues and stacks are initialized with `{}` or `{0, 1, 2}` syntax, eliminating manual `queue_create`/`stack_create` calls
2. **Buffered Collections** — Fixed-capacity queues/stacks with `queue<T>(N)` syntax for blocking channel semantics
3. **Pointer Semantics** — Raw pointers (`->queue<T>`) and fat pointers (`=>queue<T>`) for referencing collections
4. **Zero-Copy Slices** — Read-only live views of queues/stacks using `{start:end}` syntax
5. **Explicit Dereferencing** — Clear `.>` and `(->)` syntax for method calls through pointers

---

## Queues and Stacks as First-Class Types

Queues and stacks are now primitive collection types in Tig, with automatic allocation and initialization.

### Unbuffered (Dynamic) Queues and Stacks

Unbuffered collections have dynamic capacity and grow automatically when full.

#### Syntax

```tc
queue<i32> q = {0, 1, 2, 3}
stack<i32> s = {0, 2, 4, 6}
queue<f32> empty_q = {}
stack<f32> empty_s = {}
```

#### Initialization Behavior

- With initializer list `{0, 1, 2}`: The compiler allocates the collection with default capacity (e.g., 10) and automatically emits `push` calls for each element
- With empty initializer `{}`: The compiler allocates the collection with default capacity
- Without initializer: The compiler allocates the collection with default capacity

#### Generated C Code

```tc
// Tig code
queue<i32> q = {0, 1, 2}
```

```c
// Generated C code
Queue *q = queue_create(10);
queue_push(q, (void *)(intptr_t)0);
queue_push(q, (void *)(intptr_t)1);
queue_push(q, (void *)(intptr_t)2);
```

#### Methods

Queues and stacks support `.push()` and `.pop()` methods:

```tc
queue<i32> q = {0, 1, 2}
q.push(3)
i32 x = q.pop()
i32 y = q.pop()
```

---

## Buffered (Fixed-Capacity) Queues and Stacks

Buffered collections have fixed capacity and block when full or empty, providing channel-like behavior for concurrent programming.

### Syntax

```tc
queue<i32>(10) buffered_queue = {1, 2, 3, 4, 5}
stack<i32>(5) buffered_stack = {1, 2, 3}
```

### Capacity Specification

The capacity `N` in `queue<T>(N)` or `stack<T>(N)` specifies the fixed size of the collection:
- If the collection is full, `.push()` blocks until another thread calls `.pop()`
- If the collection is empty, `.pop()` blocks until another thread calls `.push()`

### Blocking Semantics

Buffered collections are thread-safe and implement blocking behavior:

```tc
queue<i32>(10) buffered_queue = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
buffered_queue.push(11)  // BLOCKS: buffer is full, waits for pop
```

### Use Case: Async Communication

Buffered queues replace channels for async function communication:

```tc
async fn void producer: queue<i32> result, i32 value {
    result.push(value * 2)
}

fn void main: {
    queue<i32>(10) q = {}
    asyncfn(q, 5)
    printi(q.pop())  // Receives 10
}
```

---

## Pointers to Queues and Stacks

Tig supports raw pointers to queues and stacks using the `->` prefix operator.

### Syntax

```tc
->queue<i32> q_ptr = &q
->stack<i32> s_ptr = &s
```

### C Representation

Since `queue<i32>` compiles to `Queue *`, a pointer to a queue compiles to `Queue **`:

```tc
// Tig code
queue<i32> q = {0, 1, 2, 3, 5}
->queue<i32> q_ptr = &q
```

```c
// Generated C code
Queue *q = queue_create(10);
queue_push(q, (void *)(intptr_t)0);
queue_push(q, (void *)(intptr_t)1);
queue_push(q, (void *)(intptr_t)2);
queue_push(q, (void *)(intptr_t)3);
queue_push(q, (void *)(intptr_t)5);
Queue **q_ptr = &q;
```

### Method Calls on Pointers

Tig requires explicit dereferencing for method calls through pointers:

#### Option 1: Pointer Field Access (`.>`)

```tc
q_ptr.>push(10)
s_ptr.>push(12)
```

#### Option 2: Prefix Dereference (`(->)`)

```tc
(->q_ptr).push(10)
(->s_ptr).push(12)
```

Both forms compile to dereferencing the pointer before calling the method:

```c
// Generated C for q_ptr.>push(10)
queue_push(*q_ptr, (void *)(intptr_t)10);
```

---

## Fat Pointers and Slices

Fat pointers (`=>T`) represent slices or views over collections. For queues and stacks, slices are **read-only live views** that reference the parent collection without copying data.

### Syntax

```tc
=>queue<i32> q_fat_ptr = q{0:2}
=>stack<i32> s_fat_ptr = s{0:2}
```

### Slice Notation

The `{start:end}` syntax creates a slice view:
- `start`: Logical index from the front of the queue (or bottom of stack)
- `end`: Logical end index (exclusive)

### Why Curly Braces?

Tig uses `{start:end}` for collection slices (distinct from array slices `[start:end]`) to:
- Distinguish logical collection views from raw memory indexing
- Signal that the slice is a view, not a contiguous memory block
- Handle ring-buffer wrap-around semantics transparently

### Read-Only Live Views

Slices of queues and stacks are **read-only live views**:
- Zero-copy: No data is copied when creating a slice
- Live: If the parent collection changes, the slice reflects those changes
- Ring-buffer safe: Handles circular queue wrap-around correctly

### C Representation

A queue slice is represented as a struct holding a reference to the parent queue and bounds:

```c
typedef struct {
    Queue *parent;
    int32_t start;
    int32_t len;
} tc_fat_queue_i32;
```

```tc
// Tig code
queue<i32> q = {0, 1, 2, 3, 5}
=>queue<i32> q_fat_ptr = q{0:2}
```

```c
// Generated C code
Queue *q = queue_create(10);
queue_push(q, (void *)(intptr_t)0);
queue_push(q, (void *)(intptr_t)1);
queue_push(q, (void *)(intptr_t)2);
queue_push(q, (void *)(intptr_t)3);
queue_push(q, (void *)(intptr_t)5);
tc_fat_queue_i32 q_fat_ptr = {
    .parent = q,
    .start = 0,
    .len = 2
};
```

### Slice Access

Elements in a slice can be accessed via indexing:

```tc
=>queue<i32> slice = q{0:2}
i32 first = slice[0]  // Access first element in slice
i32 second = slice[1] // Access second element in slice
```

---

## Complete Example: 1.3.1 Features

```tc
use "stdlib/io.tc"
use "stdlib/async.tc"

// Async function using queue for result
async fn void asyncfn: queue<i32> result, i32 value {
    result.push(value * 2)
}

// Sync function returning a stack
fn stack<i32> multiple: stack<i32> result, i32 val {
    i32 x = val + val
    i32 y = val * val
    result.push(x)
    result.push(y)
    ret result
}

fn void main: {
    // Basic queue and stack usage
    queue<i32> q = {0, 1, 2}
    q.push(3)
    i32 x = q.pop()
    i32 y = q.pop()
    printi(x)
    printi(y)

    stack<i32> s = {0, 2, 4, 6}
    i32 a = s.pop()
    i32 b = s.pop()
    printi(a)
    printi(b)

    // Async communication
    queue<i32> q2 = {}
    asyncfn(q2, 5)
    printi(q2.pop())

    // Multiple return values via stack
    stack<i32> st
    multiple(st, 5)
    i32 value1 = st.pop()
    i32 value2 = st.pop()
    printi(value1)
    printi(value2)

    // Buffered collections (fixed capacity, blocking)
    stack<i32>(10) buffered_stack = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
    queue<i32>(10) buffered_queue = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}

    // Non-buffered (dynamic capacity)
    stack<f32> nonbuffered_stack = {}
    queue<f32> nonbuffered_queue = {}

    // Pointers to collections
    queue<i32> q3 = {0, 1, 2, 3, 5}
    stack<i32> s3 = {0, 1, 2, 3, 4}

    ->queue<i32> q_ptr = &q3
    ->stack<i32> s_ptr = &s3

    // Explicit dereferencing for method calls
    q_ptr.>push(10)
    s_ptr.>push(12)

    // Fat pointers / slices (read-only live views)
    =>queue<i32> q_fat_ptr = q3{0:2}
    =>stack<i32> s_fat_ptr = s3{0:2}
}
```

---

## Type System Summary

### Collection Types

| Tig Type | C Equivalent | Description |
|----------|--------------|-------------|
| `queue<T>` | `Queue *` | Dynamic queue (unbuffered) |
| `queue<T>(N)` | `Queue *` | Fixed-capacity queue (buffered) |
| `stack<T>` | `Stack *` | Dynamic stack (unbuffered) |
| `stack<T>(N)` | `Stack *` | Fixed-capacity stack (buffered) |
| `->queue<T>` | `Queue **` | Pointer to queue |
| `->stack<T>` | `Stack **` | Pointer to stack |
| `=>queue<T>` | `tc_fat_queue_T` | Fat pointer / slice view of queue |
| `=>stack<T>` | `tc_fat_stack_T` | Fat pointer / slice view of stack |

---

## Blocking Behavior Comparison

| Collection Type | Push Behavior | Pop Behavior |
|-----------------|---------------|-------------|
| Unbuffered `queue<T>` | Never blocks (grows if needed) | Blocks if empty (in async context) |
| Buffered `queue<T>(N)` | Blocks if full | Blocks if empty |
| Unbuffered `stack<T>` | Never blocks (grows if needed) | Blocks if empty (in async context) |
| Buffered `stack<T>(N)` | Blocks if full | Blocks if empty |

---

## Error Messages

### E014: Cannot take address of temporary

```tc
->queue<i32> ptr = &queue<i32>{0, 1, 2}  // ERROR
```

```
error[E014]: cannot take address of temporary
 --> main.tc:5:25
   |
5 |     ->queue<i32> ptr = &queue<i32>{0, 1, 2}
   |                         ^^^^^^^^^^^^^^^^^^^^ cannot take address of temporary
```

### E015: Slice bounds out of range

```tc
queue<i32> q = {0, 1, 2}
=>queue<i32> slice = q{0:10}  // ERROR: only 3 elements
```

```
error[E015]: slice bounds out of range
 --> main.tc:3:31
   |
3 |     =>queue<i32> slice = q{0:10}
   |                               ^^^ end index 10 exceeds queue size 3
```

### E016: Cannot modify read-only slice

```tc
=>queue<i32> slice = q{0:2}
slice.push(5)  // ERROR: slice is read-only
```

```
error[E016]: cannot modify read-only slice
 --> main.tc:3:6
   |
3 |     slice.push(5)
   |       ^ cannot modify read-only slice
```

---

## Keywords for 1.3.1

No new keywords are introduced in 1.3.1. All features use existing syntax:
- `->` — Raw pointer prefix (existing, now applies to queues/stacks)
- `=>` — Fat pointer prefix (existing, now applies to queues/stacks)
- `{}` — Initializer list syntax (existing, now applies to queues/stacks)
- `{start:end}` — Slice syntax (new for queues/stacks)
- `.>` — Pointer field access (existing, now applies to queue/stack methods)

---

## Migration Guide from 1.3.0

### Breaking Changes

1. **Channels replaced by queues/stacks**: The `chan T` type is removed in favor of `queue<T>` and `stack<T>`
2. **Channel send/receive syntax removed**: The `<-` operator for channels is removed; use `.push()` and `.pop()` instead

### Migration Steps

#### Before (1.3.0 with channels)

```tc
chan i32 results
async producer(results)
i32 value = <-results
```

#### After (1.3.1 with queues)

```tc
queue<i32> results = {}
async producer(results)
i32 value = results.pop()
```

#### Before (1.3.0 with buffered channels)

```tc
chan i32(10) buffered
```

#### After (1.3.1 with buffered queues)

```tc
queue<i32>(10) buffered = {}
```

### New Capabilities

1. **Automatic initialization**: No more `queue_create()` or `stack_create()` calls
2. **Pointer semantics**: Pass queues/stacks by pointer with `->queue<T>` and `->stack<T>`
3. **Zero-copy slices**: Create read-only views with `=>queue<T>` and `{start:end}` syntax
4. **Explicit dereferencing**: Clear `.>` and `(->)` syntax for method calls through pointers

---

## Runtime Implementation Notes

### Thread Safety

Queues and stacks must be thread-safe when used in async contexts:
- Use mutexes for internal synchronization
- Use condition variables for blocking behavior on buffered collections
- Ensure atomic operations for size checks and index updates

### Ring Buffer Implementation

Queues should be implemented as circular ring buffers:
- Front and rear indices wrap around the buffer boundary
- Slice views must handle wrap-around by querying the parent queue's indices
- Avoid copying when creating slices; use parent reference + bounds

### Memory Management

- Queues and stacks are heap-allocated via `queue_create()` and `stack_create()`
- The compiler emits automatic cleanup for local collections (via `defer` or at scope end)
- Fat pointers do not own the parent collection; they are non-owning views

---

## Future Plans (1.4+)

1. **Networked queues** — Distributed queues across machines
2. **Work-stealing scheduler** — Advanced async task scheduling
3. **Stack/queue methods** — Additional methods like `.peek()`, `.len()`, `.clear()`
4. **Slice mutability** — Optional mutable slices with explicit opt-in
5. **Array slicing** — Unify array and collection slice syntax

---

## Summary

Tig 1.3.1 transforms queues and stacks from runtime utilities into first-class language primitives with automatic initialization, pointer semantics, and zero-copy slicing. This release simplifies concurrent programming while maintaining the low-level control and performance that Tig is known for.
