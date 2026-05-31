# Tight-C 1.3.0 Specification: Concurrency and Ownership

## Overview

Tight-C 1.3.0 introduces a comprehensive concurrency and ownership model designed to prevent data races while maintaining simplicity. The core principles are:

1. **Single ownership** - Data is owned by one thread at a time
2. **Explicit transfer** - The `@` operator explicitly moves data between threads
3. **Immutable sharing** - The `pin` keyword enables safe read-only sharing
4. **No return values** - Async functions use channels for communication

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

- **Calling an async function**: Just call it normally - it spawns a goroutine
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

By default, data is owned by the thread that created it. Async goroutines cannot borrow data - they must explicitly receive ownership.

### The `@` Operator (Give)

The `@` operator transfers ownership of data to an async function. After giving, the data is dead in the current scope.

```tc
async fn void process: i32 value {
    printi(value)
}

fn void main: {
    i32 x = 42
    async process(@x)  // x is given to process
    // x is now dead here
    printi(x)         // ERROR: x is dead in this scope
}
```

### Error: Giving Without `@`

```tc
async fn void process: i32 value {
    printi(value)
}

fn void main: {
    i32 x = 42
    async process(x)  // ERROR: x is owned by main, use @ to give
}
```

---

## Pinning (Immutable Sharing)

The `pin` keyword makes data read-only in the current scope, allowing it to be safely shared across multiple async functions without transferring ownership.

### Syntax

```tc
fn void main: {
    i32 x = 100
    pin x
    // x is now read-only in this scope
    // and can be shared with other threads
    async worker1(x)
    async worker2(x)
    async worker3(x)

    x = 12  // ERROR: x is pinned and read-only
}
```

### Pinning vs Giving

| Operation | Ownership | Mutability | Reusable in Scope |
|-----------|-----------|------------|------------------|
| `async foo(x)` | ❌ Error | N/A | N/A |
| `async foo(@x)` | Moved | Mutable | ❌ No |
| `pin x; async foo(x)` | Shared | Immutable | ✅ Yes |

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

### Pattern 1: Single Transfer (Give)

Use when you want to move data to another thread permanently:

```tc
async fn void consumer: i32 data {
    printi(data)
}

fn void main: {
    i32 x = 42
    async consumer(@x)  // x moved to consumer
    // x is dead here
}
```

### Pattern 2: Multiple Consumers (Pin)

Use when multiple threads need read-only access:

```tc
async fn void reader1: i32 data {
    printi(data)
}

async fn void reader2: i32 data {
    printi(data)
}

fn void main: {
    i32 x = 42
    pin x
    async reader1(x)  // Shared read-only
    async reader2(x)  // Shared read-only
    // x still available, read-only
}
```

### Pattern 3: Error - Double Give

```tc
async fn void worker1: i32 data {}
async fn void worker2: i32 data {}

fn void main: {
    i32 x = 42
    async worker1(@x)  // x moved, compiler copies x to heap, worker gets pointer
    async worker2(@x)  // ERROR: x is already dead
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

### Giving data in async calls

```tc
async fn void worker1: i32 data {
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

## Complete Example

```tc
use "stdlib/io.tc"

async fn void worker: i32 id, chan i32 out {
    out <- id * 2
}

fn void main: {
    chan i32 results

    pin i32 i = 0
    loop if (i < 5) {
        async worker(i, results)
        i = i + 1
    }

    pin i32 j = 0
    loop if (j < 5) {
        i32 result = <-results
        printi(result)
        print("\n")
        j = j + 1
    }
}
```

---

## Error Messages

### E010: Cannot access dead variable
```
error[E010]: cannot access dead variable 'x'
 --> main.tc:10:5
   |
10 |     printi(x)
   |         ^ cannot access dead variable 'x'
```

### E011: Cannot give owned variable without @
```
error[E011]: cannot give owned variable without '@'
 --> main.tc:8:12
   |
8 |     async foo(x)
   |            ^ use '@x' to give ownership
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

## Future Plans (1.4+)

1. Struct migration for hot reload with async
2. Networked channels
3. Work-stealing scheduler
4. Async/await syntax sugar

