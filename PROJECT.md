# Tig Programming Language 🦁

## Overview

Tig is a minimalistic systems programming language designed to be the simplest possible, yet usable, language for systems programming. It transpiles to C11 and runs anywhere C runs, making it highly portable while providing modern ergonomics without runtime overhead.

**Version:** 1.3.1  
**Language:** C11 (transpiled from Tig)  
**License:** MIT  
**Platforms:** Windows, Linux, macOS  

---

## What is Tig?

Tig is a source-to-source compiler (transpiler) written in approximately 4,000 lines of C. It reads `.tc` files and outputs portable C11 code that can be compiled with gcc, clang, or any conforming C compiler.

### Design Philosophy

- **Simplicity First**: Only 15 keywords. The entire compiler can be understood in a single afternoon.
- **No Hidden Magic**: No garbage collection, no type inference, no shadowing, no implicit allocations.
- **Explicit is Better**: Every line of Tig maps to obvious C code. You can inspect the generated C at any time.
- **Zero Runtime Overhead**: Runtime is only linked when actually needed (e.g., for async functions).
- **Portability**: Output is pure C11 with no platform-specific extensions.

### Project Goals

1. **Make the first mainstream systems language from Mexico**
2. **Explore the bare minimum of what a systems language must have to be usable, modern, and ergonomic**
3. **Keep it small enough to fit in a single person's head**

---

## Key Features

### Core Language Features

- **15 Keywords**: `if`, `loop`, `break`, `defer`, `ret`, `strun`, `fn`, `use`, `pub`, `pin`, `match`, `else`, `enum`, `async`, `select`
- **Manual Memory Management**: `alloc()` / `free()` with `defer` for guaranteed cleanup
- **Raw Pointers (`->`)**: Traditional C-style pointers for direct memory access
- **Fat Pointers (`=>`)**: Pointers with built-in length information for safer array/slice operations
- **Packed Structs**: No padding, predictable memory layout (essential for embedded systems)
- **C FFI**: Direct interop with C libraries via `extern "C"` blocks
- **Inline Imports**: `@use "lib.tc"` inlines another `.tc` file at compile time
- **CLI Arguments**: Built-in support for command-line argument parsing

### Advanced Features

#### Zero-Boilerplate Async
Write concurrent code with zero setup. No manual `async_init()` or `async_shutdown()` needed—the runtime initializes automatically on first async call.

```tig
async fn void worker: i32 x {
    printi(x)
}

fn void main: {
    worker(42)  // Automatically initializes runtime
}
```

#### Concurrent Data Structures
Built-in `queue<T>` and `stack<T>` types for thread-safe concurrent programming.

```tig
async fn void producer: queue<i32> q {
    q.push(100)
}

async fn void consumer: queue<i32> q {
    i32 value = q.pop()
    printi(value)
}
```

#### Select Statements
Wait on multiple async operations with `select`, similar to Go's select statement.

```tig
use "stdlib/io.tc"
use "stdlib/snq.tc"

fn i32 main: {
    print("Testing select statement\n")
    
    // Test 1: Basic select with queues
    queue<i32> q1 = {}
    queue<i32> q2 = {}
    
    // Add some values to queues
    q1.push(10)
    q2.push(20)
    
    print("Created queues and added values\n")
    
    // Basic select statement
    select {
        q1.pop() => {
            print("Selected from queue 1\n")
        },
        q2.pop() => {
            print("Selected from queue 2\n")
        }
    }
    
    print("Select statement test completed\n")
    ret 0
}

```

#### Ownership Transfer
The `@` operator transfers ownership of resources between async tasks safely.

#### Pin Keyword
Keep variables alive across async boundaries and prevent reassignment in specific scopes for safety.

#### Hot Reloading
Modify functions, structs, and enums on-the-fly without restarting your application. Tig uses a robust Host/DLL architecture to guarantee clean reloads.

---

## How It Works

### Compilation Pipeline

```
source.tc → [Lexer] → [Parser] → [AST] → [Emitter] → output.c → gcc/clang → binary
```

1. **Lexer**: Tokenizes source into identifiers, literals, keywords, and symbols
2. **Parser**: Builds an Abstract Syntax Tree (AST) from tokens
3. **Emitter**: Walks the AST and outputs C11 code
4. **C Compiler**: Compiles the generated C to a native binary

### What Tig is NOT

- No optimizer
- No intermediate representation (IR)
- No type inference pass
- No code generation beyond string concatenation of C

The output is always **readable, debuggable C** that you can inspect with `tigc source.tc -o source.c`.

---

## Type System

Tig provides a comprehensive set of primitive types that map directly to C types:

| Tig  | C Equivalent | Description          |
|------|--------------|----------------------|
| `i8` | `char`       | 8-bit signed integer |
| `i16`| `int16_t`    | 16-bit signed integer|
| `i32`| `int32_t`    | 32-bit signed integer|
| `i64`| `int64_t`    | 64-bit signed integer|
| `u8` | `uint8_t`    | 8-bit unsigned integer|
| `u16`| `uint16_t`   | 16-bit unsigned integer|
| `u32`| `uint32_t`   | 32-bit unsigned integer|
| `u64`| `uint64_t`   | 64-bit unsigned integer|
| `f32`| `float`      | 32-bit floating point |
| `f64`| `double`     | 64-bit floating point |
| `void`| `void`      | No type              |

### Strunions (Layout Polymorphism)

Tig combines structs and unions into a single construct called "strunions" (`strun`). This provides a spectrum from pure structs to pure unions:

```
[struct] — [strun] — [union]
```

Use `&` to create union elements inside a strunion:

```tig
// Pure struct
strun Point {
    i32 x,
    i32 y
}

// Pure union
strun Data {
    &i32 data
    &str ip
}

// Hybrid (both struct and union)
strun Hybrid {
    i32 x
    i32 y
    &i32 z  // Shares memory with w
    &f32 w
}
```

---

## Syntax Overview

### Hello World

```tig
use "stdlib/io.tc"

fn void main: {
    print("hello, world")
}
```

### Variables and Functions

```tig
i32 x = 10
f64 pi = 3.14

fn i32 add: i32 a, i32 b {
    ret a + b
}
```

### Control Flow

```tig
if (x > 0) {
    print("positive")
} else if (x < 0) {
    print("negative")
} else {
    print("zero")
}

loop {
    if (i >= 10) { break }
    printi(i)
    i = i + 1
}
```

### Memory Management

```tig
->i32 arr = alloc(i32, 100)
defer { free(arr) }  // Guaranteed cleanup
```

### Pointers

```tig
i32 x = 42
->i32 ptr = @x          // Raw pointer (address-of)
->ptr = 99              // Dereference

i32[4] arr = {1, 2, 3, 4}
=>i32 slice = @arr       // Fat pointer from array
printi(slice.len)        // Built-in length
printi(slice.ptr[0])     // Access elements

=>i32 sub = arr[1:3]     // Slicing
```

### C Interop

```tig
extern "C" {
    i32 fn printf: ->i8 fmt, ... {}
}
```

---

## Standard Library

Tig includes a comprehensive standard library organized by functionality:

### I/O (`stdlib/io.tc`)
- `print(s)`, `printn(s)` — String output
- `printi(n)`, `printin(n)` — Integer output
- `readi()`, `readc()` — Input from stdin
- File I/O via extern "C" functions

### Strings (`stdlib/str.tc`)
- `slen(s)` — String length
- `seq(a, b)` — String equality
- `scpy(dest, src)` — Copy string
- `scat(dest, src)` — Concatenate strings
- `sfind(s, c)` — Find character

### Math (`stdlib/math.tc`)
- `iabs(x)`, `min(a, b)`, `max(a, b)` — Basic math
- `clamp(x, lo, hi)` — Clamp to range
- `sqrt64(x)`, `pow64(base, exp)` — Advanced math
- Trig functions: `sin`, `cos`, `tan`

### Memory (`stdlib/mem.tc`)
- `zero(ptr, n)` — Zero out bytes
- `copy(dest, src, n)` — Copy bytes
- `memeq(a, b, n)` — Compare bytes
- `fill(ptr, val, n)` — Fill with value

### Conversions (`stdlib/conv.tc`)
- `stoi(s)` — String to integer
- `stof(s)` — String to float
- `itos(n, buf, size)` — Integer to string
- `ftos(n, buf, size)` — Float to string

### Async (`stdlib/async.tc`)
- Thread pool management
- Async function support
- Concurrent data structures

---

## Use Cases

Tig is particularly well-suited for:

- **CLI Tools**: Parse arguments, process files, call system APIs
- **Embedded / Bare-Metal**: No runtime, no allocator required, predictable memory layout
- **Game Engine Internals**: Manual memory, no GC pauses, direct pointer control
- **Learning Compilers**: Small enough to read in an afternoon, real enough to produce working binaries
- **C Codebases**: Better ergonomics (fat pointers, defer, slicing) without leaving the C ecosystem

---

## Getting Started

### Installation

```bash
# Clone the repository
git clone <repository-url>
cd tc

# Build the compiler
make

# Compile stdlib headers (only needed once)
./tigc stdlib/io.tc -o stdlib/io.h
```

### Compiling Programs

```bash
# One-step: transpile + compile to binary
./tigc samples/fizzbuzz.tc -c fizzbuzz
./fizzbuzz

# Two-step: transpile to C, then compile yourself
./tigc samples/fizzbuzz.tc -o fizzbuzz.c
gcc fizzbuzz.c -std=c11 -o fizzbuzz
```

### Compiler Flags

| Flag | Description |
|------|-------------|
| `-o file.c` | Emit transpiled C to file (`.h` gets `#pragma once`) |
| `-c binary` | Transpile + compile to binary (auto-detects gcc/clang) |
| `-t` | Keep temporary files for debugging |
| `-H <libname>` | Enable hot reload mode |
| `--hot` | Rebuild hot library for running application |

---

## Error Reporting

Tig provides Rust-style colored diagnostics with source lines and carets:

```
error[E000]: cannot assign to pinned variable 'x'
 --> samples/pin.tc:8:5
  |
8 |     x = 11 // this should be illegal since x is pinned in this scope
  |     ^ cannot assign to pinned variable 'x'

E000
Type "tigc --error E000" for help
```

---

## Inspiration

Tig draws inspiration from several languages:

- **Pony**: Actor-based concurrency and safety
- **Nim**: Minimal syntax and C transpilation
- **Go**: Simplicity and concurrency primitives
- **C**: Direct memory control and portability
- **Rust**: Error messages and safety features

---

## Project Structure

```
tc/
├── compiler/
│   ├── include/     # Header files
│   └── src/         # Compiler source (C)
├── stdlib/          # Standard library (.tc files)
├── samples/         # Example programs
├── demos/           # Non-trivial examples
├── DOCUMENTATION/   # Language specification
└── Makefile         # Build system
```

---

## License

MIT License - see LICENSE file for details.

---

## Contributing

Tig is an open-source project. Contributions are welcome! Please see CONTRIBUTING.md for guidelines.

---

## Author

Built by [alonsovm44](https://github.com/alonsovm44)

---

## Sponsor

Support the project on [Ko-fi](https://ko-fi.com/alonsovm)
