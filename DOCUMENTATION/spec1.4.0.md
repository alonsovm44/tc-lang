# Tig version 1.4.0 Specification

## Overview
Tig 1.4.0 introduces a comprehensive memory management evolution, building upon the manual memory model established in versions 1.2 and 1.3. This release provides developers with multiple memory management strategies—arena allocation, optional garbage collection, and enhanced ownership semantics—while maintaining backward compatibility with existing manual memory management patterns.

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
    i32[] buffer = alloc(1024)  // Allocated from arena
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

## Garbage Collection

### Thesis
Tig provides an optional, scoped garbage collection mechanism for scenarios where manual memory management would be impractical. The GC is opt-in and scoped, allowing developers to selectively apply automatic memory management to specific code regions while maintaining manual control elsewhere.

### When to Use GC
Garbage collection is recommended for:
- **Long-running processes** where memory leaks would be catastrophic
- **Complex data structures** with intricate ownership graphs (e.g., cyclic references, shared caches)
- **Rapid prototyping** where manual memory management would slow development

### Syntax

```tig
use gc {
    // Code within this block uses GC-managed memory
}
```

### Semantics

**Scope Isolation:**
- Data allocated (stack or heap) within a `use gc` block is tracked by the garbage collector
- GC-tracked data cannot escape the GC block; pointers to GC-managed memory cannot be stored in variables that outlive the block
- Data from outside GC blocks is readable within GC blocks (read-only access)

**Memory Scope Contract:**
- GC blocks, like arena blocks, are memory scopes
- Pointers allocated inside cannot be returned or stored in outer-scope variables
- This ensures the GC can safely reclaim all tracked memory when the block exits

**Implementation Notes:**
- The GC is inactive by default; only code within `use gc` blocks participates in garbage collection
- GC runs automatically when memory pressure thresholds are reached or when the GC block exits
- The GC algorithm is a simple, conservative collector optimized for Tig's type system

## Data Movement and Ownership Transfer

### Thesis
Tig introduces channel-based ownership transfer to safely move data between memory scopes. This mechanism enables data to flow from GC or arena blocks into manual memory zones, and between async blocks, while maintaining clear ownership semantics and preventing use-after-free errors.

### Channel Syntax

- **Send operator (`@`)**: Transfers ownership of a value into a channel
- **Receive operator (`<-`)**: Receives ownership from a channel

### Basic Scope Transfer

```tig
{ // Scope 1
    i32 x = 10
    chan i32 ch = @x  // Ownership of x transferred to channel
}  // x is no longer accessible here

{ // Scope 2
    i32 y = <-ch  // Ownership received from channel
    // y is now available in this scope
}
```
### GC to Manual Transfer

```tig
// Global scope (manual memory management)
use gc {
    fn i32 foo(i32 x) {
        ret x * 2
    }
    
    i32 x = foo(5)  // x is GC-managed
    // x is available within this GC block only

    // Transfer ownership to channels
    chan i32 ch = @x
    chan fn(i32)i32 fn_ch = @foo
    // x and foo cannot be reused here—ownership transferred
}
// Manual memory zone

i32 y = <-ch  // y is now manually managed
fn(i32)i32 fn_y = <-fn_ch
i32 z = fn_y(10)  // Execute transferred function
```

### Arena to Manual Transfer

```tig
arena[1024] {
    i32 x = 10  // x is arena-allocated
    // x is available within this arena block only

    chan i32 ch = @x  // Transfer ownership to channel
}

i32 y = <-ch  // y is now manually managed
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
** Pinning to share data to multiple scopes**
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
Tig allows runtime selection of memory management strategies based on environmental conditions. This enables adaptive behavior—using GC for complex scenarios, arenas for performance-critical paths, or manual management for predictability—within a single codebase.

### Syntax

```tig
if (condition) {
    use gc {
        // GC-managed memory path
    }
} else {
    arena[size] {
        // Arena-managed memory path
    }
}
```

### Use Cases

**Environment-based selection:**
```tig
if (is_development_build()) {
    use gc {
        // Simplify debugging with GC in development
    }
} else {
    arena[calculate_arena_size()] {
        // Optimize for performance in production
    }
}
```

**Data-size-based selection:**
```tig
if (data_size > LARGE_DATA_THRESHOLD) {
    use gc {
        // GC for large, complex structures
    }
} else {
    // Manual management for small, simple data
}
```

### Trade-offs
- **Flexibility**: Code can adapt to runtime conditions, enabling optimization for different scenarios
- **Predictability**: Runtime selection makes behavior dependent on execution environment
- **Testing**: Multiple code paths require testing each memory management strategy

**Design Philosophy**: The unpredictability introduced by conditional memory management is intentional—it provides a mechanism for adaptive software that can optimize for different deployment scenarios rather than being locked into a single strategy.

## Asynchronous Execution

### Thesis
Async blocks provide concurrent execution by default. Data isolation between async blocks is enforced, and channels enable safe data movement between concurrent contexts.

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
async {
    i32 x = 10
    chan i32 ch = @x  // Send x to channel
}

async {
    i32 y = <-ch  // Receive from channel
    // Process y
}
```

### Synchronization

While async blocks run concurrently, synchronization mechanisms (channels, barriers, or explicit wait primitives) should be used to coordinate access to shared resources and ensure deterministic program behavior.

## Language Additions in 1.4.0

### New Keywords
1. **`arena`** - Introduces arena allocation scope
2. **`gc`** - Enables garbage collection scope via `use gc`

### New Operators
1. **`@`** - Channel send operator (ownership transfer)
2. **`<-`** - Channel receive operator (ownership acquisition)

### New Block Types
1. **`arena[size] { }`** - Arena allocation block
2. **`use gc { }`** - Garbage collection block
3. **`async { }`** - Asynchronous execution block

## Migration Guide

### From 1.3 to 1.4
- Existing code using manual memory management remains unchanged
- Arena and GC features are opt-in; no migration required for existing codebases
- Channel-based ownership transfer is additive; existing pointer patterns continue to work

### Recommended Adoption Path
1. **Start with arenas** for performance-critical sections with known allocation patterns
2. **Introduce GC** for complex data structures where manual management is error-prone
3. **Use async blocks** for I/O-bound or parallelizable workloads
4. **Leverage conditional memory management** for adaptive deployment strategies
