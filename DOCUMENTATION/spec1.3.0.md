# Tig 1.3.0 Specification: Concurrency and Ownership

## Overview

Tig 1.3.0 introduces a comprehensive concurrency and ownership model designed to prevent data races while maintaining simplicity. The core principles are:

1. **Single ownership** - Data is owned by one thread at a time
2. **Explicit transfer** - The `@` operator explicitly moves data between threads
3. **Immutable sharing** - The `pin` keyword enables safe read-only sharing
4. **No return values** - Async functions use channels (queues/stacks) for communication

---

## Async Functions

Functions can be marked as `async` to indicate they run in a separate goroutine.

### Syntax

```tc
async fn void foo: {
    // async function body
}
```

### Calling Async Functions

- **Calling an async function**: Just call it normally, it spawns a goroutine
- **Calling a sync function asyncly**: Use the `async` keyword before the call

```tc
async fn void worker: {
    print("Worker running\n")
}

fn void task: {
    print("Task running\n")
}

fn void main: {
    worker()           // Spawns goroutine
    async task()       // Spawns goroutine for sync function
}
```

---

## Ownership Model

By default, data is owned by the thread that created it. Tig's ownership system prevents data races by ensuring that mutable data is never shared between threads without explicit synchronization.

### Type Categories and Ownership Rules

Tig v1.3.0 classifies types into three categories with different ownership semantics:

1. **Value Types** (Primitives: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32`, `f64`, `bool`):
   - Passed by value (implicitly copied) to both sync and async functions
   - No ownership transfer needed - each copy is independent
   - Safe to pass without `@` or `pin`

2. **Reference Types** (Channels: `queue<T> / stack<T>`):
   - Thread-safe concurrent queues/channels with internal synchronization
   - Passing a channel copies its reference handle, not its ownership
   - Safe to pass to multiple async functions without `@` or `pin`

3. **Owned Types** (Pointers: `->T`, `=>T`, and structs containing pointers):
   - Unique resources that must be explicitly managed
   - MUST be moved with `@` (transfer ownership) or shared with `pin` (immutable sharing)
   - This prevents shared mutable state and data races

### The `@` Operator (Give)

The `@` operator transfers ownership of owned types to an async function. After giving, the data is dead in the current scope and cannot be accessed.

```tc
async fn void process: ->i32 data {
    printi(->data)
}

fn void main: {
    i32 x = 42
    ->i32 ptr = &x
    async process(@ptr)  // ptr ownership moved to process
    // ptr is now dead here
    printi(->ptr)         // ERROR: ptr is dead in this scope
}
```

**Note**: For value types (primitives), `@` is not needed since they are copied automatically:

```tc
async fn void process: i32 value {
    printi(value)
}

fn void main: {
    i32 x = 42
    async process(x)  // OK: x is copied (value type)
    printi(x)         // OK: x still available
}
```

### Error: Giving Owned Type Without `@`

```tc
async fn void process: ->i32 data {
    printi(->data)
}

fn void main: {
    i32 x = 42
    ->i32 ptr = &x
    async process(ptr)  // ERROR: ptr is owned by main, use @ to give
}
```

---

## Pinning (Immutable Sharing)

The `pin` keyword makes data read-only in the current scope, allowing it to be safely shared across multiple async functions without transferring ownership. This is primarily useful for owned types (pointers and structs with pointers).

### Syntax

```tc
fn void main: {
    i32 x = 100
    ->i32 ptr = &x
    pin ptr
    // ptr is now read-only in this scope
    // and can be shared with other threads
    async worker1(ptr)
    async worker2(ptr)
    async worker3(ptr)

    ptr = &x  // ERROR: ptr is pinned and read-only
}
```

**Note**: For value types (primitives), `pin` is not needed since they are copied automatically when passed to async functions.

### Pinning vs Giving

| Operation | Ownership | Mutability | Reusable in Scope | Type Applicability |
|-----------|-----------|------------|------------------|-------------------|
| `async foo(x)` (value type) | Copied | Mutable | ✅ Yes | Value types only |
| `async foo(x)` (owned type) | ❌ Error | N/A | N/A | Error for owned types |
| `async foo(@x)` | Moved | Mutable | ❌ No | Owned types only |
| `pin x; async foo(x)` | Shared | Immutable | ✅ Yes | Owned types only |

---

## Sync Function Calls

Regular (non-async) functions can receive data by value or by reference.

### Pass by Value (Copy)

```tc
fn void foo: i32 x {
    printi(x)
}

fn void main: {
    i32 x = 42
    foo(x)  // OK: x is copied
    printi(x)  // OK: x still available
}
```

### Pass by Reference (Borrow)

```tc
fn void modify: ->i32 x {
    ->x = 100
}

fn void main: {
    i32 x = 42
    modify(&x)  // OK: x is borrowed
    printi(x)  // Prints 100
}
```

---

## Sharing Data Patterns

### Pattern 1: Value Type Sharing (Automatic Copy)

For primitives, data is automatically copied when passed to async functions:

```tc
async fn void consumer: i32 data {
    printi(data)
}

fn void main: {
    i32 x = 42
    async consumer(x)  // x is copied, no @ needed
    printi(x)          // OK: x still available
}
```

### Pattern 2: Owned Type Transfer (Give)

Use when you want to move owned data (pointers) to another thread permanently:

```tc
async fn void consumer: ->i32 data {
    printi(->data)
}

fn void main: {
    i32 x = 42
    ->i32 ptr = &x
    async consumer(@ptr)  // ptr moved to consumer
    // ptr is dead here
}
```

### Pattern 3: Owned Type Sharing (Pin)

Use when multiple threads need read-only access to owned data:

```tc
async fn void reader1: ->i32 data {
    printi(->data)
}

async fn void reader2: ->i32 data {
    printi(->data)
}

fn void main: {
    i32 x = 42
    ->i32 ptr = &x
    pin ptr
    async reader1(ptr)  // Shared read-only
    async reader2(ptr)  // Shared read-only
    // ptr still available, read-only
}
```

### Pattern 4: Error - Double Give

```tc
async fn void worker1: ->i32 data {}
async fn void worker2: ->i32 data {}

fn void main: {
    i32 x = 42
    ->i32 ptr = &x
    async worker1(@ptr)  // ptr moved
    async worker2(@ptr)  // ERROR: ptr is already dead
}
```

---

## Return Values

Async functions **cannot return values**. This is by design to prevent race conditions and simplify the model.

### Invalid: Async with Return Value

```tc
async fn i32 compute: {  // ERROR: async functions cannot return values
    ret 42
}
```

### Valid: Async Void

```tc
async fn void worker: {
    print("Working...\n")
}
```

### Giving Owned Data in Async Calls

```tc
async fn void worker1: ->i32 data {
    async worker2(@data) // data is moved to worker2 and dead in this scope
}
```

### For the 20% Case: Use Channels

For cases where you need results from async functions, use channels (see below).

---

## Channels

Channels provide a way for async functions to communicate and send results back.

### Creating Channels

```tc
chan i32 results
```

### Sending to Channels

```tc
async fn void producer: chan i32 out {
    out <- 42
}
```

### Receiving from Channels

```tc
fn void main: {
    chan i32 results
    async producer(results)
    i32 value = <-results
    printi(value)
}
```

### Buffered Channels

```tc
chan i32(10) buffered  // Buffer size of 10
```

### Select on Multiple Channels

```tc
fn void main: {
    chan i32 ch1
    chan i32 ch2

    select {
        <-ch1 => {
            print("Received from ch1\n")
        }
        <-ch2 => {
            print("Received from ch2\n")
        }
    }
}
```

---

## Complete Example & Design Solutions

### The Loop Concurrency Challenge

When launching async tasks in a loop, we face two main issues with a strict single-ownership model:
1. **Loop Index Mutation**: If we pin `i` to share it, we cannot increment `i++`. If we move `i` with `@i`, it becomes dead and we cannot continue the loop.
2. **Channel Sharing**: If passing a channel transfers its ownership, the channel is dead after the first iteration and cannot be reused.

### The Elegant Solution: Type-Based Ownership

The loop concurrency challenge is solved elegantly by Tig's type-based ownership system:

1. **Value Types** (Primitives: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32`, `f64`, `bool`):
   - Passed by value (implicitly copied) to both sync and async functions
   - Since a copy is created on the new thread's stack, there is no ownership transfer and no data race
   - Therefore, loop indices like `i` do not need `@` or `pin`, and can be mutated normally

2. **Reference Types** (Channels: `chan T`):
   - Channels are thread-safe concurrent queues with internal synchronization
   - Passing a channel copies its reference handle, not its ownership
   - Therefore, a channel can be safely passed to multiple async functions in a loop without being consumed

3. **Owned Types** (Pointers: `->T`, `=>T`, and structs containing pointers):
   - These are unique resources that must be explicitly managed
   - MUST be moved with `@` (transfer ownership) or shared with `pin` (immutable sharing)
   - This prevents shared mutable heap state and data races

### Valid Loop Example

```tc
use "stdlib/io.tc"

async fn void worker: i32 id, queue<i32> out {
    out.push(id * 2)
}

fn void main: {
    queue<i32> results = {}// Shared channel/blocking queue (copied handle)

    i32 i = 0        // Primitive type (automatically copied)
    loop if (i < 5) {
        worker(i, results) // i is copied, results handle is copied. Safe!
        i++
    }

    i32 j = 0
    loop if (j < 5) {
        i32 result = results.pop()
        printi(result)
        print("\n")
        j++
    }
}
```

---

## Error Messages

### E010: Cannot access dead variable
```
error[E010]: cannot access dead variable 'ptr'
 --> main.tc:10:5
   |
10 |     printi(->ptr)
   |         ^ cannot access dead variable 'ptr'
```

### E011: Cannot give owned variable without @
```
error[E011]: cannot give owned variable without '@'
 --> main.tc:8:12
   |
8 |     async foo(ptr)
   |            ^ use '@ptr' to give ownership
```

### E012: Async functions cannot return values
```
error[E012]: async functions cannot return values
 --> main.tc:5:1
   |
5 | async fn i32 compute: {
   | ^^^^^^^^^^^^^^^^^^^^^^ use channels instead
```

### E013: Cannot assign to pinned variable
```
error[E013]: cannot assign to pinned variable 'x'
 --> main.tc:9:5
   |
9 |     x = 12
   |     ^ cannot assign to pinned variable
```

---

## Keywords for 1.3.0

1. `async` - Mark function as async (new)
2. `@` - Give ownership to async function (new)
3. `chan` - Channel type (new)
4. `<-` - Channel send/receive operator (new)
5. `select` - Channel selection (new)
6. `pin` - Immutable variable (existing, enhanced for concurrency)

---

## Migration Guide from 1.2.0

### Breaking Changes

None - async features are opt-in.

### New Capabilities

- Add `async` before function declarations to make them async
- Use `@` to transfer data to async functions
- Use `pin` to share data immutably
- Use channels for async communication

---

## Future Plans (1.5+)

1. Struct migration for hot reload with async
2. Networked channels
3. Work-stealing scheduler
4. Async/await syntax sugar

