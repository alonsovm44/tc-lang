# Tig Language Specification

## Overview

Tig is a systems programming language designed to be as simple as possible while remaining highly usable. It compiles to C and provides low-level control with modern ergonomics, making it ideal for game development, embedded systems, and high-performance applications.

### Design Philosophy

- **No GC** — Manual memory management only (for now)
- **No complex safety** — Safety through explicitness, not hidden checks
- **No type inference** — Everything is explicit and clear
- **No shadowing** — Pick names well and avoid confusion
- **No OOP** — Imperative procedural programming
- **No indentation-based syntax** — Braces only
- **No C++-style references** — Avoid reference-related bugs
- **No string type** — C-like strings (null-terminated `->u8`)
- **Math built-in** — Full set of operators
- **Zero defaults** — Uninitialized variables default to 0
- **Manual scopes** — Explicit block scoping
- **Single return values** — Functions return one value (use queues/stacks for multiple returns)
- **No integer promotion** — Explicit casting required
- **Overflow wraps** — Standard two's complement wrap-around

### Language Goals

1. **Simplicity** — Minimal syntax, clear semantics
2. **Performance** — Zero-cost abstractions, direct C compilation
3. **Control** — Manual memory management with optional safety features
4. **Productivity** — Hot reloading, ergonomic syntax, powerful standard library

---

## Table of Contents

1. [Lexical Structure](#lexical-structure)
2. [Types](#types)
3. [Variables and Scoping](#variables-and-scoping)
4. [Functions](#functions)
5. [Control Flow](#control-flow)
6. [Data Structures](#data-structures)
7. [Pointers and Memory](#pointers-and-memory)
8. [Queues and Stacks](#queues-and-stacks)
9. [Concurrency](#concurrency)
10. [Hot Reloading](#hot-reloading)
11. [C FFI and Inline Code](#c-ffi-and-inline-code)
12. [Macros](#macros)
13. [Modularity](#modularity)
14. [Methods](#methods)
15. [Built-in Functions](#built-in-functions)
16. [Operators](#operators)
17. [Error Messages](#error-messages)
18. [Compiler Usage](#compiler-usage)
19. [Migration Guide](#migration-guide)

---

## Lexical Structure

### Comments

```tc
// Single-line comment

/* Multi-line
   comment */
```

### Identifiers

Identifiers start with a letter or underscore, followed by letters, digits, or underscores:
```tc
variable_name
_private_var
MAX_VALUE
```

### Keywords

```
if, else, loop, break, defer, ret, strun, fn, use, pub, pin, match,
async
```

---

## Types

### Primitive Types

| Tig Type | C Equivalent | Size | Description |
|----------|--------------|-------|-------------|
| `void` | `void` | - | No type |
| `i8` | `int8_t` | 1 byte | Signed 8-bit integer |
| `i16` | `int16_t` | 2 bytes | Signed 16-bit integer |
| `i32` | `int32_t` | 4 bytes | Signed 32-bit integer |
| `i64` | `int64_t` | 8 bytes | Signed 64-bit integer |
| `u8` | `uint8_t` | 1 byte | Unsigned 8-bit integer |
| `u16` | `uint16_t` | 2 bytes | Unsigned 16-bit integer |
| `u32` | `uint32_t` | 4 bytes | Unsigned 32-bit integer |
| `u64` | `uint64_t` | 8 bytes | Unsigned 64-bit integer |
| `f32` | `float` | 4 bytes | 32-bit floating point |
| `f64` | `double` | 8 bytes | 64-bit floating point |
| `bool` | `bool` | 1 byte | Boolean (true/false) |

### Pointer Types

| Tig Type | C Equivalent | Description |
|----------|--------------|-------------|
| `->T` | `T *` | Raw pointer to T |
| `=>T` | `tc_fat_T` | Fat pointer (slice) to T |

### Collection Types

| Tig Type | C Equivalent | Description |
|----------|--------------|-------------|
| `T[N]` | `T[N]` | Fixed-size array of N elements |
| `queue<T>` | `Queue *` | Dynamic queue (unbuffered) |
| `queue<T>(N)` | `Queue *` | Fixed-capacity queue (buffered) |
| `stack<T>` | `Stack *` | Dynamic stack (unbuffered) |
| `stack<T>(N)` | `Stack *` | Fixed-capacity stack (buffered) |
| `->queue<T>` | `Queue **` | Pointer to queue |
| `->stack<T>` | `Stack **` | Pointer to stack |
| `=>queue<T>` | `tc_fat_queue_T` | Fat pointer / slice view of queue |
| `=>stack<T>` | `tc_fat_stack_T` | Fat pointer / slice view of stack |

### Function Pointer Types

```tc
fn(param_types)return_type
```

Example:
```tc
fn(i32, i32)i32  // Function pointer taking two i32, returning i32
```

### Enum Types

```tc
enum Name {
    MEMBER1
    MEMBER2
    ...
}
```

Enums default to `i32` in C.

---

## Variables and Scoping

### Variable Declaration

```tc
<type> <name> = <value>
<type> <name>  // Defaults to 0
```

Examples:
```tc
i32 x = 10
i32 y        // y is 0
f32 pi = 3.14
```

### Uninitialized Variables

All variables default to 0 if not explicitly initialized:
```tc
i32 x          // x is 0
i32[5] arr     // All elements are 0
```

### Pinning (Immutability)

The `pin` keyword makes a variable immutable in the current scope:

```tc
pin <var>
```

Example:
```tc
i32 x = 100
pin x
// x is now read-only in this scope
x = 200  // ERROR: cannot assign to pinned variable
```

### Manual Scoping

Scopes are defined with braces `{ }`:

```tc
{
    // scope 1
    {
        // scope 1.1
    }
    {
        // scope 1.2
    }
}
{
    // scope 2
}
```

Inner scopes can access variables from outer scopes, but not vice versa.

---

## Functions

### Function Declaration

```tc
[pub] [async] fn <return_type> <name>: <type> <arg>, ... {
    // body
    ret <value>
}
```

Examples:
```tc
fn void main: {
    ret
}

pub fn i32 add: i32 a, i32 b {
    ret a + b
}

fn ->Point create_point: i32 x, i32 y {
    // ...
}
```

### Function Parameters

Parameters are passed by value (copied) by default. Use pointers to pass by reference:

```tc
fn void modify: ->i32 x {
    ->x = 100
}

fn void main: {
    i32 value = 10
    modify(&value)
    printi(value)  // Prints 100
}
```

### Return Values

Functions return a single value. For multiple return values, use queues or stacks:

```tc
fn stack<i32> multiple: stack<i32> result, i32 val {
    i32 x = val + val
    i32 y = val * val
    result.push(x)
    result.push(y)
    ret result
}
```

### Function Pointers

Function pointers enable callbacks and higher-order functions:

```tc
fn i32 add: i32 a, i32 b {
    ret a + b
}

fn i32 callback: fn(i32, i32)i32 fptr, i32 a, i32 b {
    ret fptr(a, b)
}

fn void main: {
    i32 result = callback(add, 5, 3)
    printi(result)  // Prints 8
}
```

### Varargs

Functions can declare variadic arguments using `...`:

```tc
extern "C" {
    i32 fn printf: ->i8 fmt, ... {}
}
```

Usage:
```tc
printf("Hello %s, number: %d\n", "world", 42)
```

---

## Control Flow

### If Statements

```tc
if (condition) {
    // code
} else if (condition) {
    // code
} else {
    // code
}
```

Example:
```tc
fn void check_grade: i32 score {
    if (score >= 90) {
        print("A\n")
    } else if (score >= 80) {
        print("B\n")
    } else if (score >= 70) {
        print("C\n")
    } else {
        print("F\n")
    }
}
```

### Loop Statements

```tc
loop {
    // infinite loop unless break
    break
}

loop if (condition) {
    // conditional loop
}
```

Example:
```tc
i32 i = 0
loop if (i < 10) {
    printi(i)
    i++
}
```

### Match Statements

Pattern matching with wildcard support:

```tc
match (value) {
    1 = {
        print("one\n")
    }
    2 = {
        print("two\n")
    }
    _ = {
        print("other\n")  // wildcard (catch-all)
    }
}
```

### Defer

Deferred execution at scope exit:

```tc
defer {
    // code executes at end of scope
}
```

Example:
```tc
fn void example: {
    ->i32 ptr = alloc(i32, 100)
    defer { free(ptr) }
    // use ptr...
}  // ptr is automatically freed here
```

---

## Data Structures

### Strunions (Struct/Union Hybrid)

Strunions combine structs and unions into a single construct:

```tc
strun <name> {
    <type> <field>      // struct field (separate memory)
    &<type> <field>     // union field (shared memory)
}
```

#### Regular Struct (All normal fields)

```tc
strun Point {
    i32 x
    i32 y
}
```

#### Union (All `&`-prefixed fields)

```tc
strun Data {
    &i32 data
    &->i8 ip
}
```

#### Hybrid (Mixed struct and union)

```tc
strun Hybrid {
    i32 x
    i32 y
    &i32 z
    &f32 w
}
```

#### Memory Grouping

Anonymous fields separate union blocks:

```tc
strun Data {
    &i32 x
    &i32 y
    i32        // Anonymous padding (4 bytes)
    &f32 z
    &f64 a
    u8         // Anonymous padding (1 byte)
    &->i8 c
    &i8 d
}
```

#### Packed Strunions

By default, strunions are not packed. Use `@` to pack:

```tc
@strun PackedData {
    // packed data
}
```

### Enums

Simple enums for type-safe constants:

```tc
enum Color {
    RED
    GREEN
    BLUE
    BLACK
    WHITE
}
```

Usage:
```tc
Color c = RED
if (c == RED) {
    print("Color is RED\n")
}
```

### Arrays

Fixed-size arrays:

```tc
<type>[<size>] <name> = {<values>}
```

Example:
```tc
i32[5] arr = {0, 1, 2, 3, 4}
i32[10] empty  // All elements default to 0
```

Array indexing:
```tc
i32 x = arr[0]
arr[1] = 42
```

---

## Pointers and Memory

### Raw Pointers

```tc
-><type>    // pointer type
&<var>      // address-of
-><ptr>     // dereference
```

Example:
```tc
i32 x = 10
->i32 ptr = &x
i32 value = ->ptr  // value is 10
```

### Pointers as Arrays

Raw pointers can be indexed like arrays:

```tc
->i32 ptr = alloc(i32, 10)
ptr[0] = 1
ptr[1] = 2
```

### Fat Pointers (Slices)

Fat pointers represent slices with length information:

```tc
=><type> <name> = arr[<start>:<end>]
```

Example:
```tc
i32[7] arr = {0, 1, 2, 3, 4, 5, 6}
=>i32 slice = arr[1:4]

i32 len = lenof(slice)  // 3
i32 first = slice[0]    // 1
```

### Strings

String literals decay to `->u8`:

```tc
->u8 hello = "hello"     // pointer to first character
u8 first = ->hello       // 'h'
u8 second = hello[1]     // 'e'
```

Fat pointer strings with length:
```tc
=>u8 hello = "hello"
u8 size = lenof(hello)  // 5
u8 last = hello[size - 1]  // 'o'
```

### Manual Memory Allocation

```tc
alloc(<type>, <count>)  // allocate count elements of type
free(<ptr>)             // free allocated memory
```

Example:
```tc
->i32 arr = alloc(i32, 100)
defer { free(arr) }
```

### Pointer Field Access (`.>`)

To access struct fields through pointers, use `.>`:

```tc
strun Person {
    i32 age
    ->i8 name
}

fn void foo: ->Person p {
    p.>age = 23
    p.>name = "John"
}
```

Alternative explicit dereferencing:
```tc
(->p).age = 23
```

---

## Queues and Stacks

Queues and stacks are first-class collection types with automatic initialization.

### Unbuffered (Dynamic) Collections

Dynamic capacity, grows automatically when full:

```tc
queue<i32> q = {0, 1, 2, 3}
stack<i32> s = {0, 2, 4, 6}
queue<f32> empty_q = {}
```

### Buffered (Fixed-Capacity) Collections

Fixed capacity with blocking semantics:

```tc
queue<i32>(10) buffered_queue = {1, 2, 3, 4, 5}
stack<i32>(5) buffered_stack = {1, 2, 3}
```

Blocking behavior:
- `.push()` blocks if collection is full
- `.pop()` blocks if collection is empty

### Methods

```tc
q.push(value)  // push value
i32 x = q.pop()  // pop value
i32 x = q.peek()  // peek at front without removing
i32 n = q.size()  // get current size
q.clear()  // remove all elements
bool empty = q.isEmpty()  // check if empty
```

### Automatic Cleanup

Queue and stack variables are automatically destroyed when they go out of scope, freeing their internal memory:

```tc
fn void example: {
    queue<i32> q = {1, 2, 3}
    // use q...
}  // q is automatically destroyed here (queue_destroy called)
```

### Pointers to Collections

```tc
->queue<i32> q_ptr = &q
->stack<i32> s_ptr = &s
```

Method calls on pointers (explicit dereferencing):
```tc
q_ptr.>push(10)
(->q_ptr).push(10)
```

### Slices (Fat Pointers)

Read-only live views using `{start:end}`:

```tc
=>queue<i32> q_fat_ptr = q{0:2}
=>stack<i32> s_fat_ptr = s{0:2}
```

Slices are zero-copy and handle ring-buffer wrap-around correctly.

---

## Concurrency

### Async Functions

Functions marked with `async` run in separate goroutines:

```tc
async fn void worker: {
    print("Worker running\n")
}

fn void main: {
    worker()  // Spawns goroutine
}
```

### Calling Sync Functions Asyncly

```tc
fn void task: {
    print("Task running\n")
}

fn void main: {
    async task()  // Spawns goroutine for sync function
}
```

### Ownership Model

Tig classifies types into three categories:

1. **Value Types** (Primitives): Passed by value, no ownership transfer needed
2. **Reference Types** (Queues/Stacks): Thread-safe, copied handle
3. **Owned Types** (Pointers): Must be moved with `@` or shared with `pin`

### The `@` Operator (Give)

Transfers ownership of owned types to async functions:

```tc
async fn void process: ->i32 data {
    printi(->data)
}

fn void main: {
    i32 x = 42
    ->i32 ptr = &x
    async process(@ptr)  // ptr ownership moved
    // ptr is dead here
}
```

### Pinning (Immutable Sharing)

Makes data read-only for safe sharing:

```tc
fn void main: {
    i32 x = 100
    ->i32 ptr = &x
    pin ptr
    async worker1(ptr)
    async worker2(ptr)
    // ptr still available, read-only
}
```

### Async Functions with Queues

Async functions cannot return values. Use queues for communication:

```tc
async fn void producer: queue<i32> q {
    q.push(100)
    q.push(200)
}

async fn void consumer: queue<i32> q {
    i32 value1 = q.pop()
    i32 value2 = q.pop()
    printi(value1)
    printi(value2)
}
```

### Async Blocks

Async blocks provide concurrent execution:

```tc
async {
    // Code executes concurrently
}
```

---

## Hot Reloading

### Global Hot Reloading

All code (except `main`) moves to a shared library for hot reloading:

```bash
# Normal compilation
tigc source.tc -c app

# Global hot reload
tigc source.tc -c app -H hotlib
./app

# Hot rebuild while running
tigc source.tc -H hotlib --hot
```

### Architecture

- **Host Executable**: Contains `main` function and reload stubs
- **Hot Library**: Contains all other code (functions, struns, enums)
- **Dynamic Reloading**: Host detects library changes and reloads automatically

### Benefits

- Modify functions, struns, and enums on-the-fly
- No application restart required
- Preserves program state during reload
- Automatic cleanup of old library versions

---

## C FFI and Inline Code

### Extern C Declarations

```tc
extern "C" {
    <return_type> fn <name>: <type> <arg>, ... {}
}
```

Example:
```tc
extern "C" {
    i32 fn printf: ->i8 fmt, ... {}
    ->FILE fn fopen: ->i8 file, ->i8 mode {}
    i32 fn fclose: ->FILE stream {}
}
```

### Inline C Code

Direct C code injection:

```tc
"C"{
    // C code here
    printf("Hello from inline C!\n");
}
```

Accessing Tig variables:
```tc
fn void main: {
    i32 tc_var = 100
    "C"{
        printf("Tig variable: %d\n", tc_var);
        tc_var = 200;
    }
    printi(tc_var)  // Prints 200
}
```

### Inline C Headers

Include C headers globally:

```tc
"C"{
    #include "libs/raylib.h"
}
```

---

## Macros

### Basic Macros

Text replacement without parameters:

```tc
# name{replacement}
```

Example:
```tc
# int{i32}
# float{f32}

fn void main: {
    int x = 10      // expands to: i32 x = 10
    float y = 3.14  // expands to: f32 y = 3.14
}
```

### Parametric Macros

Macros with parameters:

```tc
# name(param1, param2, ...){replacement}
```

Example:
```tc
# swap(T, x, y){
    T temp = x
    x = y
    y = temp
}

fn void main: {
    i32 a = 1
    i32 b = 2
    swap(i32, a, b)
}
```

### Macro Scoping

Macros are scoped to their declaration block:

```tc
# float{f32}  // Global scope

fn void foo: {
    # int{i32}  // Local to foo
}
```

---

## Modularity

### Imports

```tc
use "path/to/module.tc"
```

### Inline Imports

Inline `.tc` files without `.h` counterpart:

```tc
@use "lib.tc"
```

### Public Visibility

All objects are private by default. Use `pub` to export:

```tc
pub fn i32 add: i32 a, i32 b {
    ret a + b
}

pub strun Point {
    i32 x
    i32 y
}

pub i32 global_var = 10
```

---

## Methods

Struns can contain methods with implicit `self` parameter:

```tc
strun Person {
    i32 age
    ->i8 name

    fn void sayHi: {
        print("Hi from Person!\n")
    }

    fn void setAge: i32 newAge {
        self.>age = newAge
    }

    fn i32 getAge: {
        ret self.>age
    }
}
```

### Method Calls

```tc
Person person
person.age = 25
person.name = "Alice"
person.sayHi()
person.setAge(30)
i32 currentAge = person.getAge()
```

### Generated C

Methods compile to C functions with struct pointer as first parameter:

```c
void Person_sayHi(struct Person *self) {
    printf("Hi from Person!\n");
}

int32_t Person_getAge(struct Person *self) {
    return self->age;
}
```

### Method Naming Convention

Generated C functions follow: `StructName_methodName`

---

## Built-in Functions

### Type and Size Operations

```tc
sizeof(<type>)   // Returns size in bytes
lenof(<array>)   // Returns element count
cast(<expr>, <type>)  // Explicit cast
```

### I/O Functions (stdlib/io.tc)

| Function | Description |
|----------|-------------|
| `print(s)` | Print string + newline |
| `printn(s)` | Print string, no newline |
| `printi(n)` | Print i64 + newline |
| `printin(n)` | Print i64, no newline |
| `readi()` | Read i64 from stdin |
| `readc()` | Read single char from stdin |

---

## Operators

### Bitwise

```
&   // and
|   // or
^   // xor
!   // not
<<  // shl (shift left)
>>  // shr (shift right)
```

### Arithmetic Operators

```
+   // addition
-   // subtraction
*   // multiplication
/   // division
%   // modulo
++  // increment
--  // decrement
+=  // add-assign
-=  // subtract-assign
*=  // multiply-assign
/=  // divide-assign
%=  // modulo-assign
```

### Comparison Operators

```
==  // equal
<>  // not equal
<   // less than
>   // greater than
<=  // less than or equal
>=  // greater than or equal
```

### Logical Operators

```
&&  // logical and
||  // logical or
```

### Overflow Semantics

Integer overflow wraps (two's complement):
```tc
u8 x = 255
x++  // wraps to 0
```

### Explicit Casting

No implicit conversions. Use `cast()`:
```tc
i32 x = cast(i32, 10.4)
```

Rounding:
- Decimal part < 0.5: floors
- Decimal part >= 0.5: ceilings

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

### E014: Cannot take address of temporary

```
error[E014]: cannot take address of temporary
 --> main.tc:5:25
   |
5 |     ->queue<i32> ptr = &queue<i32>{0, 1, 2}
   |                         ^^^^^^^^^^^^^^^^^^^^ cannot take address of temporary
```

### E015: Too few arguments to function

```
error[E015]: Too few arguments to function 'add': expected 2, got 1
 --> main.tc:7:5
   |
7 |     add(5)
   |     ^^^ Too few arguments to function 'add': expected 2, got 1
```

### E016: Too many arguments to function

```
error[E016]: Too many arguments to function 'add': expected 2, got 3
 --> main.tc:7:5
   |
7 |     add(1, 2, 3)
   |     ^^^ Too many arguments to function 'add': expected 2, got 3
```

### E017: Owned variable must be passed with @

```
error[E017]: Owned variable 'x' must be passed with @ operator
 --> main.tc:7:5
   |
7 |     consume(x)
   |     ^ Owned variable 'x' must be passed with @ operator
```

---

## Compiler Usage

### Basic Compilation

```bash
tigc <input.tc> [-o output.c] [-c binary]
```

### Flags

| Flag | Description |
|------|-------------|
| `-o file.c` | Emit transpiled C to file |
| `-c binary` | Transpile + compile to binary |
| `-H <libname>` | Enable hot reload mode |
| `--hot` | Rebuild only hot library |
| `-t, --temp` | Keep temporary `.c` files |

### Examples

```bash
# Transpile to stdout
tigc main.tc

# Transpile to file
tigc main.tc -o main.c

# Compile to binary
tigc main.tc -c app

# Hot reload mode
tigc main.tc -H hotlib -c app

# Hot rebuild
tigc main.tc -H hotlib --hot
```

### Building the Compiler

```bash
make          # Build tigc
make clean    # Remove build artifacts
```

---

## Migration Guide

### From 1.3.0 to 1.3.1

**Channels replaced by queues/stacks:**

Before:
```tc
chan i32 results
async producer(results)
i32 value = <-results
```

After:
```tc
queue<i32> results = {}
async producer(results)
i32 value = results.pop()
```

**Buffered channels to buffered queues:**

Before:
```tc
chan i32(10) buffered
```

After:
```tc
queue<i32>(10) buffered = {}
```

### From 1.2.x to 1.3.0

**Add async features:**
- Add `async` before function declarations
- Use `@` to transfer data to async functions
- Use `pin` to share data immutably
- Use queues for async communication

### From 1.1.x to 1.2.x

**Hot reloading:**
- Use `-H <libname>` flag for hot reload mode
- Use `--hot` flag for hot rebuild
- No `hot` keyword needed in code

---

## Appendix: Complete Example

```tc
use "stdlib/io.tc"
use "stdlib/async.tc"

// Async function using queue
async fn void producer: queue<i32> q {
    q.push(100)
    q.push(200)
}

// Sync function returning stack
fn stack<i32> multiple: stack<i32> result, i32 val {
    i32 x = val + val
    i32 y = val * val
    result.push(x)
    result.push(y)
    ret result
}

// Strun with methods
strun Counter {
    i32 value

    fn void init: {
        self.>value = 0
    }

    fn void increment: {
        self.>value = self.>value + 1
    }

    fn i32 getValue: {
        ret self.>value
    }
}

fn void main: {
    // Queue and stack usage
    queue<i32> q = {0, 1, 2}
    q.push(3)
    i32 x = q.pop()
    printi(x)

    // Async communication
    queue<i32> q2 = {}
    producer(q2)
    printi(q2.pop())

    // Multiple return values via stack
    stack<i32> st
    multiple(st, 5)
    i32 value1 = st.pop()
    i32 value2 = st.pop()
    printi(value1)
    printi(value2)

    // Buffered collections
    queue<i32>(10) buffered_q = {1, 2, 3}

    // Pointers to collections
    ->queue<i32> q_ptr = &q
    q_ptr.>push(10)

    // Slices
    =>queue<i32> slice = q{0:2}

    // Methods
    Counter counter
    counter.init()
    counter.increment()
    printi(counter.getValue())

    // Inline C
    "C"{
        printf("Hello from inline C!\n");
    }
}
```

---

## Version History

### 1.3.2
- Added queue/stack methods: size(), clear(), isEmpty(), peek()
- Automatic cleanup for queue/stack variables on scope exit
- Function argument count validation (E015, E016)
- Move semantics enforcement with @ operator (E017)
- Queue/stack runtime functions moved to stdlib/snq.c for proper linking

### 1.3.1
- First-class queues and stacks with automatic initialization
- Buffered collections with blocking semantics
- Pointer semantics for collections
- Zero-copy slices (read-only live views)

### 1.3.0
- Async functions and goroutines
- Ownership model with `@` operator
- Pinning for immutable sharing
- Channels for async communication

### 1.2.3
- Bulletproof hot reloading architecture
- Host/DLL splitting for stable reloads

### 1.2.2
- Text replacement macros
- Global hot reloading (no `hot` keyword)
- sizeof, typeof, alignof, offsetof

### 1.2.1
- Enums
- else/else if statements
- Inline C code
- Pointer field access (`.>`)
- Methods in struns

### 1.2.0
- Hot function reloading
- Varargs support
- Expanded I/O stdlib

### 1.1.0
- Function pointers
- Match statements
- Strunions (struct/union hybrid)
- Pointers to structs
- Function syntax improvement

### 1.0.0
- Core language features
- Variables, functions, structs
- Loops, pointers, arrays
- Manual memory management
- C FFI
