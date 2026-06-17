# Tig version 1.4.0 Specification

## Overview
Tig 1.4.0 introduces a comprehensive memory management evolution, building upon the manual memory model established in versions 1.2 and 1.3. This release provides developers with multiple memory management strategies—arena allocation, and enhanced ownership semantics—while maintaining backward compatibility with existing manual memory management patterns.

## Current Memory Model (Pre-1.4)
Prior to version 1.4, Tig employs a C-like manual memory management model:
- No borrow checker or compile-time ownership verification
- No garbage collection
- No arena allocation
- Safety mechanisms limited to LIFO-style `defer{}` blocks for cleanup

## Arena Allocation

### Thesis
Arena allocation provides scoped, bulk memory management with deterministic cleanup. All allocations within an arena are freed atomically when the arena scope exits, eliminating per-allocation deallocation overhead and reducing memory fragmentation.

### Syntax

```tig
arena[size] {
    // Allocations within this scope use arena memory
}
```

The `size` parameter specifies the arena capacity in bytes and may be:
- A compile-time constant expression
- A runtime variable (evaluated at arena construction)

### Examples

**Constant-sized arena:**
```tig
arena[32 * 1024] {  // 32KB arena
    =>i32 buffer = alloc(1024)  // Allocated from arena
    // ... buffer usage ...
}  // All arena memory freed here
```

**Variable-sized arena:**
```tig
usize required_size = calculate_required_memory()
arena[required_size] {
    // Arena sized to runtime requirements
}
```

### Semantics
- **Scope-bound lifetime**: All memory allocated within an arena block is reclaimed when the block exits, regardless of exit path (normal return, early return, or error)
- **No individual deallocation**: `free()` operations are unnecessary within arena scopes
- **Allocation failure**: If arena capacity is exhausted, allocation results in a runtime panic
- **Nesting**: Arena blocks may be nested; inner arenas are independent of outer arenas
- **Zero-overhead**: Arena allocation incurs minimal runtime cost compared to manual heap allocation 

### Thesis
Tig extends queues to support ownership transfer for moving data between memory scopes. This mechanism enables data to flow from arena blocks into manual memory zones, and between async blocks, while maintaining clear ownership semantics and preventing use-after-free errors.

### Queue-Based Ownership Transfer

- **`queue.push(@value)`**: Transfers ownership of a value into a queue
- **`queue.pop()`**: Receives ownership from a queue

### Basic Scope Transfer

```tig
{ // Scope 1
    i32 x = 10
    queue<i32> q = {}
    q.push(@x)  // Ownership of x transferred to queue
}  // x is no longer accessible here

{ // Scope 2
    i32 y = q.pop()  // Ownership received from queue
    // y is available in this scope, and has the value of x
}
```

#### Scope-Escape Rules for Transferred Functions & Closures

To guarantee that executing an escaped function/closure (`fn_y`) outside of its origin environment never causes a segfault or undefined behavior (from referencing a torn-down block or Arena), the compiler enforces a compile-time **Scope-Escape Rule** based on the closure's capture environment:

1. **Pure Functions (Zero Capture)**:
   - If a function (like `foo` above) captures no variables from its outer lexical scope, it is transpiled to a raw C function pointer.
   - **Rule**: Pure functions can escape any memory scope (Arena) freely since they carry no state.

2. **Value-Type Closures (Value Capture)**:
   - If a function captures only primitive value types (`i8`, `i32`, `f32`, etc.), the compiler generates a closure environment structure and copies those values directly by value.
   - **Rule**: Value-type closures can escape freely because they hold independent copies, ensuring no references point back to the collected memory scope.

3. **Reference-Type Closures (Pointer Capture)**:
   - If a function captures any reference type, pointer (`->T`), slice (`=>T`), or object allocated inside the Arena block, the environment contains pointers to resources that will be reclaimed when the block exits.
   - **Rule**: Reference-type closures **cannot escape** the scope. The compiler's escape analysis tracks references, and calling `push(@...)` on such a closure triggers a compile-time error:
     ```
     error[E040]: closure captures arena managed references and cannot escape this scope.
     ```

### Arena to Manual Transfer

```tig
queue<i32> ch = {}

arena[1024] {
    i32 x = 10  // x is arena-allocated
    // x is available within this arena block only

    ch.push(@x)  // Transfer ownership to queue
}

i32 y = ch.pop()  // y is now manually managed
// y is available in the outer scope
```

### External Data Access

**Read-Only Access (No Ownership Transfer):**
```tig
i32 x = 10  // x is in outer scope

arena[1024] {
    // x is readable here (read-only access)
    i32 y = x  // Valid: copy value
    
    // Invalid: cannot modify without ownership
    x++  // Error: this scope does not own x
}
```

**Ownership Transfer from External Data:**
```tig
i32 x = 10  // x is in outer scope

arena[1024] {
    @x  // Transfer ownership into arena
    x++  // Now valid: arena owns x
}
// x is no longer accessible in outer scope
```

**Pinning to share data to multiple scopes**
```tig
pin i32 x = 10  // x is in outer scope

arena[1024] {
    x // x is pinned (share read-only access)
    // x can be read but not modified
    i32 y = x
}
{
    x // x is still pinned (share read-only access)
    // x can be read but not modified
    i32 z = x
}
// x is still accessible in outer scope but is read only
```

### Ownership Rules and Anti-Patterns

**Invalid: Multiple Ownership Transfers**
```tig
i32 x = 10
{
    @x  // Transfer ownership to this scope
    x++
}
{
    @x  // Error: x's ownership was already transferred
    x++
}
```

**Valid: Shared Read Access (No Transfer)**
```tig
i32 x = 10
{
    i32 y = x  // Copy value, no ownership transfer
}
{
    i32 z = x  // Copy value again, x still owned by outer scope
}
```

**Valid: Single Transfer with Read-Only Sharing**
```tig
i32 x = 10
{
    @x  // Transfer ownership
    i32 y = x++  // Modify within owning scope
}
{
    i32 z = x  // Error: x no longer accessible here
}
```

## Conditional Memory Management

### Thesis
Tig allows runtime selection of memory management strategies based on environmental conditions. This enables adaptive behavior—using arenas for performance-critical paths, or manual management for predictability—within a single codebase.

### Syntax

```tig
if (condition) {
    // manual memory management
} else {
    arena[size] {
        // Arena-managed memory path
    }
}
```


**Data-size-based selection:**
```tig
if (data_size > LARGE_DATA_THRESHOLD) {
    arena[size] {
        // arena for large, complex structures
    }
} else {
    // Manual management for small, simple data
}
```

### Trade-offs
- **Flexibility**: Code can adapt to runtime conditions, enabling optimization for different scenarios
- **Predictability**: Runtime selection makes behavior dependent on execution environment
- **Testing**: Multiple code paths require testing each memory management strategy

**Design Philosophy**: The unpredictability introduced by conditional memory management is intentional, it provides a mechanism for adaptive software that can optimize for different deployment scenarios rather than being locked into a single strategy.

## Asynchronous Execution

### Thesis
Async blocks provide concurrent execution by default. Data isolation between async blocks is enforced, and queues/stacks enable safe data movement between concurrent contexts.

### Syntax

```tig
async {
    // Code executes concurrently with caller
}
```

### Semantics
- Async blocks begin execution immediately upon creation
- The caller does not wait for async block completion unless explicitly synchronized
- Each async block has its own memory scope

### Data Movement Between Async Blocks

```tig
queue<i32> ch = {}

async {
    i32 x = 10
    ch.push(@x)  // Send x to queue
}

async {
    i32 y = ch.pop()  // Receive from queue
    // Process y
}
```

### Synchronization

While async blocks run concurrently, synchronization mechanisms (queues, barriers, or explicit wait primitives) should be used to coordinate access to shared resources and ensure deterministic program behavior.

## Language Additions in 1.4.0

### New Keywords
1. **`arena`** - Introduces arena allocation scope

### New Operators
1. **`@`** - Ownership transfer operator introdued in 1.3 (used with queue.push for sending) with new semantics for 1.4

### New Block Types
1. **`arena[size] { }`** - Arena allocation block
3. **`async { }`** - Asynchronous execution block

## Migration Guide

### From 1.3 to 1.4
- Existing code using manual memory management remains unchanged
- Arena features are opt-in; no migration required for existing codebases
- Queue-based ownership transfer is additive; existing pointer patterns continue to work

### Recommended Adoption Path
1. **Start with arenas** for performance-critical sections with known allocation patterns
3. **Use async blocks** for I/O-bound or parallelizable workloads
4. **Leverage conditional memory management** for adaptive deployment strategies


# Notes

|Summary Table
|Feature|	Verdict|	Notes|
:-------|:---------|:--------|
|Arena allocation|	✅ Keep	|Clean syntax, good semantics
|Queue ownership transfer|	✅ Keep	|Novel and elegant
|Scope-escape rules|	✅ Keep	|Well-specified
|pin for read-only|	✅ Keep	|Complements @
|Conditional memory management|	⚠️ Rethink	|Code duplication problem
|Nested arena independence|	⚠️ Clarify	|"Independent" needs definition
|Arena exhaustion handling |	⚠️ Expand	|Panic-only is limiting
|Defer inside arena	|❌ Missing	|Specify behavior
|Async + arena interaction |	❌ Missing	|Critical edge case
|Arena introspection |	❌ Missing	|arena_remaining() etc.

## Comptime execution[APPROVED]
<!--> <-->
Following the "explicit" nature of the language, comptime execution is explicit and requires the `comptime` keyword and
within comptime blocks.
```tcs
comptime {
    // Code that executes at compile time
}
```

Example
```tc
fn void make_array(int size) {
    comptime {
        "C"{
            // Code that executes at compile time
            int[] arr = new int[size];
            for (int i = 0; i < size; i++) {
                arr[i] = i;
            }
        }
    }
}
// or this

comptime {
    // Imagine this reads a config file or a JSON string
    ->u8 data = "10,20,30,40"
    
    // Generate a C array initialization
    "C"{
        int lookup_table[] = {%s};
    }
}

fn void main: {
    // lookup_table is now a real C array in your binary
    printi(lookup_table[1]) // Prints 20
}

// other example

fn void generate_adders: {
    i32 i = 0
    loop if(i<=100){
        // generate functions at comptime
        "C"{
            int add_%d(int a, int b) {
                return a + b + %d;
            }
        }
        i++
    }
}

comptime {
    generate_adders()
}

fn void main:{
    printi(add_99(1, 2)) // Prints 102
}
```

### Generate Tig code in comptime

```
comptime {
    ??? // i dont know how would this look like
}
```