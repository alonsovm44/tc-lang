<p align="center">
  <h1 align="center">Tig</h1>
  <p align="center">Simplest possible, usable systems language</p>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/version-1.3.0-blue?style=flat-square" alt="Version">
  <img src="https://img.shields.io/badge/language-C11-orange?style=flat-square" alt="Language">
  <img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" alt="License">
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat-square" alt="Platform">
</p>

---

Tig is a minimalistic systems programming language.

## What's New in v1.3.0 🚀

- **Zero-Boilerplate Async** — Write concurrent code with zero setup! No manual `async_init()` or `async_shutdown()` needed. Runtime initializes automatically on first async call.
- **Async Functions** — Simple async syntax: `async fn void worker: i32 x { printi(x) }` and call with `worker(42)`
- **Thread Pool Management** — Built-in thread pool with automatic resource management and cleanup
- **Smart Compilation** — Runtime automatically linked only when async functions are used (zero overhead for sync programs)
- **Clean Output** — No debug noise, just your program output
- **Stdlib Module** — `use "stdlib/async.tc"` for async functionality
- **Ownership Transfer** — `@` operator for transferring ownership of resources between async tasks
- **Select Statements** — Wait on multiple async operations with `select`
- **Queue & Stack Types** — Built-in concurrent data structures with `queue<i32>` and `stack<i32>` (still buggy)
- **Pin Keyword** — Keep variables alive across async boundaries with `pin`
## Project Goals
> Make the first mainstream systems langauge from Mexico.
> Explore the bare minimum of what a systems language must have to be usable, modern and ergonomic

## Features

- **15 keywords** — `if`, `loop`, `break`, `defer`, `ret`, `strun`, `fn`, `use`, `pub`, `pin`, `match`, `else`, `enum`, `async`, `select`
- **Zero-Boilerplate Async** — Automatic runtime initialization, no manual setup needed
- **Async Functions** — Simple concurrent programming with `async fn`
- **Concurrent Data Structures** — Built-in `queue<T>` and `stack<T>` types
- **Ownership Transfer** — `@` operator for safe resource transfer
- **Select Statements** — Multi-operation waiting with `select`
- **Pin Keyword** — Keep variables alive across async boundaries
- **No hidden magic** — no GC, no type inference, no shadowing, no aliasing
- **Raw pointers** (`->`) and **fat pointers** (`=>`) with built-in slicing
- **Manual memory** — `alloc()` / `free()` with `defer` for cleanup
- **Packed structs** — no padding, predictable layout
- **C FFI** — `extern "C"` for direct interop
- **Rust-style errors** — colored diagnostics with source lines and carets
- **One-step compile** — `tightc source.tc -c app` transpiles and compiles in one command
- **Inline imports** — `@use "lib.tc"` inlines another `.tc` file at compile time
- **CLI args** — `i32 fn main: =>->i8 args { ... }` for command-line tools

## Philosophy

**Why it was made**: I wanted a simple systems language with less keywords than Go, without GC and without heavy runtime overhead.
**Backend**: For now it is C11 
**Core Ideal**: Tig has to be able to fit in a single man's head. Small but powerful.

### Inspiration
- Pony
- Nim
- Go
- C
- Rust

### Core principle:
> Everything that can be built with libraries has to be built with libraries. The core language is small.
> Explicit is better than implicit, but exceptions can be made...
> Flexibility over rigidity.
> Anything C can run, we run: Anywhere C has ran, we will run


## Quick Start

Clone the repo and build the compiler:
```bash
# Build the compiler
make # requires make

# Compile stdlib headers in the stdlib folder (only needed once)
./tigc stdlib/io.tc -o stdlib/io.h

# One-step: transpile + compile to binary
./tigc samples/fizzbuzz.tc -c fizzbuzz
./fizzbuzz

# Or two-step: transpile to C, then compile yourself
./tigc samples/fizzbuzz.tc -o fizzbuzz.c
gcc fizzbuzz.c -std=c11 -o fizzbuzz
```

## How It Works

Tig is a **source-to-source compiler** (transpiler) written in ~4156 lines of C. It reads `.tc` files and outputs portable C11.

```
source.tc → [Lexer] → [Parser] → [AST] → [Emitter] → output.c → gcc/clang → binary
```

### Pipeline

1. **Lexer** (`lexer.c`) — Tokenizes source into identifiers, literals, keywords, and symbols. Tracks line/col for error reporting.
2. **Parser** (`parser.c`) — Builds an AST from tokens. Handles operator precedence, scope-level `pin` enforcement, and `@use` file inlining (recursive parse + splice).
3. **Emitter** (`emitter.c`) — Walks the AST and outputs C11. Most constructs are 1:1 with targeted transforms:

| Tig | Emitted C |
|---------|-----------|
| `=>i32 s` | `tc_fat_i32 s` (struct with `.ptr` + `.len`) |
| `defer { free(p) }` | Scope-exit statements emitted in reverse order before `}` and `return` |
| `pin x` | Nothing emitted — enforced at parse time (compile error on reassignment) |
| `@use "lib.tc"` | Declarations inlined directly into AST (no `#include`) |
| `alloc(T, n)` | `TC_ALLOC(T, n)` → `calloc(n, sizeof(T))` |
| `=>->i8 args` in main | `main(int argc, char **argv)` + local fat pointer wrapping them |

### What it is *not*

There is no optimizer, no IR, no type inference pass, and no code generation beyond string concatenation of C. The output is always **readable, debuggable C** that you can inspect with `tightc source.tc -o source.c`.

## Design Goals

| Goal | How |
|------|-----|
| **Predictability** | Every line maps to obvious C. No hidden allocations, no implicit copies, no vtables. |
| **Simplicity** | 10 keywords. The entire compiler is a single-pass parser + tree-walk emitter. |
| **Portability** | Output is C11 with no platform-specific extensions. Compiles with gcc, clang, or any conforming C compiler. |
| **Safety without runtime cost** | Fat pointers carry length at zero overhead (struct field). `pin` catches mutation bugs at compile time. `defer` prevents resource leaks. |
| **Interop** | `extern "C"` blocks let you call any C library directly. `use` includes `.h` files. The generated code is linkable from C. |

### What Tig is good for

- **CLI tools** — parse args, process files, call system APIs
- **Embedded / bare-metal** — no runtime, no allocator required, predictable memory layout (packed structs)
- **Game engine internals** — manual memory, no GC pauses, direct pointer control
- **Learning compilers** — small enough to read in an afternoon, real enough to produce working binaries
- **C codebases that want better ergonomics** — fat pointers, defer, slicing, without leaving the C ecosystem

## Hello World

```
use "stdlib/io.tc"

fn void main: {
    print("hello, world")
}
```

## Syntax Overview

### Variables
```
i32 x = 10
f64 pi = 3.14
u8 byte
```
Uninitialized variables default to `0`.

### Functions
```
fn i32 add: i32 a, i32 b {
    ret a + b
}
```

### Varargs
Functions can declare variadic arguments with `...`:
```
extern "C" {
    i32 fn printf: ->i8 fmt, ... {}
}
```
Note: `...` in function calls is implicit - you don't need to write it when calling varargs functions.

### Strunions (Layout polymorphism)
Both a struct and a union, this way we dont need two keywords for structs and unions
Now we have a spectrum.

[struct]—[strun]—[union]

Use `&` to create a union element inside the strun. 

For a normal struct:
```
strun Point{
  i32 x,
  i32 y
}
```
For a Union:
```
strun Data{
  &i32 data
  &str ip
}
```
For a strun:
```
strun hybrid{
  i32 x
  i32 y
  &i32 z
  &f32 w
}
```
`z` and `w` share the same memory location.
- Anonymous padding
You can padd memory inside struns with anonymous types
Example:
```
strun hybrid{
  &i32 x
  &i32 y
  i32 // anonymous padding
  &i32 z
  &f32 w
}
```
This groups `x` and `y` together, and `z` and `w` together.

### Pointers
```
i32 x = 42
->i32 ptr = @x          // raw pointer (address-of)
->ptr = 99              // dereference

i32[4] arr = {1,2,3,4}
=>i32 slice = @arr       // fat pointer from array
printi(slice.len)        // built-in length
printi(slice.ptr[0])     // access elements

=>i32 sub = arr[1:3]     // slicing
```

### Pointer Combos
```
->->i32 pp = @p          // pointer to pointer
=>->i32 fps = @ptrs      // fat pointer of raw pointers
->=>i32 pslice = @slice  // raw pointer to fat pointer
=>=> sslice = @slice     // fat pointer to fat pointer
```


### Strun pointers
```
strun Point{
    i32 x,
    i32 y
}

Point p = {1, 2}

fn void printP: ->Point p {
    printi(p.>x)
    printi(p.>y)

    // or this can be done too
    printi((->p).x)
    printi((->p).y)
}

```

### Control Flow
```
if (x > 0) { ... }
else if (x < 0) { ... }
else { ... }

loop { ... break }          // infinite loop unless break

loop if (i < 10) { ... }    // conditional loop
```

### Memory
```
->i32 arr = alloc(i32, 100)
defer { free(arr) }
```

### Imports
```
use "stdlib/io.tc"       // link to pre-compiled .h
@use "utils.tc"          // inline .tc at compile time
```

### CLI Arguments
```
i32 fn main: =>->i8 args {
    printi(args.len)         // argc
    print(args.ptr[1])       // first user argument
    ret 0
}
```

### C FFI
```
extern "C" {
    i32 fn printf: ->i8 fmt, ... {}
}
```

### Error Reporting
```
error[E000]: cannot assign to pinned variable 'x'
 --> samples/pin.tc:8:5
   |
 8 |     x = 11 // this should be illegal since x is pinned in this scope
   |     ^ cannot assign to pinned variable 'x'

E000
Type "tightc --error E000" for help

PS C:\Users\me\.projects\langs\tc> ./tightc --error E000
E000: Assignment to pinned variable

A variable marked with `pin` is immutable in the current scope.
You cannot reassign it with `=`, `+=`, `-=`, or any other assignment.

Bad:
    i32 x = 10
    pin x
    x = 11       // error: cannot assign to pinned variable

Fix: remove the `pin` or avoid reassigning the variable.
```
### Match stmts
```
match (n) {
        1 = {
            print("one")
        }
        2 = {
            print("two")
        }
        3 = {
            print("three")
        }
        _ = {
            print("other")
        }
    }

```
### Zero-Boilerplate Async

```tig
use "stdlib/async.tc"
use "stdlib/io.tc"

async fn void worker: i32 x {
    printi(x)
}

fn void main: {
    // No async_init() or async_shutdown() needed!
    worker(42)  // Automatically initializes runtime
}
```

### Concurrent Data Structures

```tig
use "stdlib/async.tc"
use "stdlib/io.tc"

async fn void producer: queue<i32> q {
    q.push(100)
}

async fn void consumer: queue<i32> q {
    i32 value = q.pop()
    printi(value)
}

fn void main: {
    queue<i32> q = queue_create(10)
    producer(q)
    consumer(q)
}
```

### Select Statements

```tig
async fn void task1: { printi(1) }
async fn void task2: { printi(2) }

fn void main: {
    select {
        case task1():
            printi("Task 1 completed")
        case task2():
            printi("Task 2 completed")
    }
}
```

## Types

| Tig | C Equivalent |
|---------|-------------|
| `i8`    | `char`      |
| `i16`   | `int16_t`   |
| `i32`   | `int32_t`   |
| `i64`   | `int64_t`   |
| `u8`    | `uint8_t`   |
| `u16`   | `uint16_t`  |
| `u32`   | `uint32_t`  |
| `u64`   | `uint64_t`  |
| `f32`   | `float`     |
| `f64`   | `double`    |
| `void`  | `void`      |

## Compiler Usage

```bash
tigc <input.tc> [-o output.c] [-c binary] [-t]
```

| Flag | Description |
|------|-------------|
| `-o file.c` | Emit transpiled C to file (`.h` gets `#pragma once`) |
| `-c binary` | Transpile + compile to binary (auto-detects gcc/clang) |
| `-t` | Keep temporal files|
| (none) | Print transpiled C to stdout |

Combine both: `tightc app.tc -o app.c -c app` keeps the `.c` and builds the binary.

## Hot Reloading

Tig supports global hot reloading. This allows you to modify functions, structs (`strun` definitions), and enums, and recompile shared libraries *completely on-the-fly* without restarting your running application.

### Basic Usage

```bash
# 1. Compile host + hot library version 1
tigc hot.tc -H hotlib -c hot_app

# 2. Run the application
./hot_app

# 3. While running, modify any function/logic in hot.tc and rebuild the library
tigc hot.tc -H hotlib --hot
```

The running application will automatically detect the changes, unload the old library, load the new one, and immediately execute the new code on the next loop iteration.

### How It Works

Tig uses a robust Host/DLL splitting architecture to avoid Windows file locking issues and guarantee clean reloads during loops:

- **Host (The Driver)**: The `main` function is compiled directly into the host executable. This ensures the main driver/application loop resides safely outside the shared library, avoiding trapped call stacks. The host manages loading/unloading and resolves stubs.
- **Library (The Engine)**: All other functions, structs, and enums are compiled into the shared library (`hotlib_N.dll` on Windows / `hotlib_N.so` on Unix). Every function is exported automatically.
- **Dynamic Reloading**: When a function is called, a host stub checks the current library version, reloads if a new version is detected, and executes through function pointers.
- **Automatic Cleanup**: On a successful reload, the host automatically and cleanly deletes old version files to keep your workspace pristine.

### Example

```tightc
use "stdlib/io.tc"

extern "C" {
    i32 fn Sleep: u32 ms {}
}

fn i32 add: i32 x, i32 y {
    ret x + y + 10
}

fn i32 main: {
    loop {
        i32 result = add(3, 4)
        printi(result)
        Sleep(2000)
    }
}
```

Running this prints `17` every 2 seconds. If you edit `ret x + y + 10` to `ret x + y + 20` and run `tigc hot.tc -H hotlib --hot`, the output instantly changes to `27` without restarting the app!

### Additional Flags

| Flag | Description |
|------|-------------|
| `-H <libname>` | Enable hot reload mode and specify the shared library name |
| `--hot` | Rebuild only the hot library version for a running application |
| `-t, --temp` | Keep temporary `.c` files for debugging |

### Proof of Concept

See the `demos/HOTSWAPPING/` folder for a complete working example with documentation, including the demo output showing hot reload in action.

This feature demonstrates Tig's capability for advanced systems programming patterns, using the industry-standard approach to hot reload on Windows (versioned libraries).

## Project Structure

```
tc-lang/
  compiler/
    include/     # Header files
    src/         # Compiler source (C)
  stdlib/        # Standard library (.tc)
  samples/       # Example programs
  docs/          # Language specification
  Makefile       # Build system
```

## Stdlib

**`stdlib/io.tc`** — I/O

| Function     | Description                |
|--------------|----------------------------|
| `print(s)`   | Print string + newline     |
| `printn(s)`  | Print string, no newline   |
| `printi(n)`  | Print i64 + newline        |
| `printin(n)` | Print i64, no newline      |
| `readi()`    | Read i64 from stdin        |
| `readc()`    | Read single char from stdin|
| `unreadc(c, stream)` | Push char back to file stream |
| `write_file(s, stream)` | Write string to file |
| `eof(stream)` | Check if at end of file |

**File I/O (via extern "C")**

| Function     | Description                |
|--------------|----------------------------|
| `fopen(file, mode)` | Open file |
| `fclose(f)` | Close file |
| `fgetc(stream)` | Read char from file |
| `ungetc(c, stream)` | Push char back to file |
| `fputs(s, stream)` | Write string to file |
| `fprintf(stream, fmt, ...)` | Formatted print to file |
| `fscanf(stream, fmt, ...)` | Formatted read from file |
| `feof(stream)` | Check end of file |

**`stdlib/str.tc`** — Strings

| Function              | Description                          |
|-----------------------|--------------------------------------|
| `slen(s)`             | String length                        |
| `seq(a, b)`           | String equality (returns 1 if equal) |
| `scpy(dest, src)`     | Copy string                          |
| `scat(dest, src)`     | Concatenate strings                  |
| `sneq(a, b, n)`       | Compare first n bytes                |
| `sfind(s, c)`         | Find first char occurrence           |
| `sfindlast(s, c)`     | Find last char occurrence            |
| `shas(haystack, needle)` | Find substring                    |

**`stdlib/math.tc`** — Math

| Function             | Description                       |
|----------------------|-----------------------------------|
| `iabs(x)`            | Absolute value (integer)          |
| `min(a, b)`          | Minimum of two integers           |
| `max(a, b)`          | Maximum of two integers           |
| `clamp(x, lo, hi)`   | Clamp value to range              |
| `sqrt64(x)`          | Square root (f64)                 |
| `pow64(base, exp)`   | Power (f64)                       |
| `fabs64(x)`          | Absolute value (f64)              |
| `sin`, `cos`, `tan`  | Trig functions (extern C)         |
| `log`, `log2`, `log10` | Logarithms (extern C)           |

**`stdlib/mem.tc`** — Memory

| Function              | Description                       |
|-----------------------|-----------------------------------|
| `zero(ptr, n)`        | Zero out n bytes                  |
| `copy(dest, src, n)`  | Copy n bytes (overlap safe)       |
| `memeq(a, b, n)`      | Compare n bytes (1 if equal)      |
| `fill(ptr, val, n)`   | Fill n bytes with value           |

**`stdlib/conv.tc`** — Conversions

| Function              | Description                       |
|-----------------------|-----------------------------------|
| `stoi(s)`             | String to i64                     |
| `stoib(s, base)`      | String to i64 with base           |
| `stof(s)`             | String to f64                     |
| `itos(n, buf, size)`  | i64 to string (into buffer)       |
| `ftos(n, buf, size)`  | f64 to string (into buffer)       |

---

<p align="center">
  <sub>Built by <a href="https://github.com/alonsovm44">@alonsovm44</a></sub>
</p>
