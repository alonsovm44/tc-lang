<p align="center">
  <h1 align="center">Tight-C</h1>
  <p align="center">Simplest possible, usable systems language</p>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/version-1.0.0-blue?style=flat-square" alt="Version">
  <img src="https://img.shields.io/badge/language-C11-orange?style=flat-square" alt="Language">
  <img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" alt="License">
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat-square" alt="Platform">
  <img src="https://img.shields.io/github/repo-size/alonsovm44/tc-lang?style=flat-square" alt="Repo Size">
</p>

---

Tight-C is a minimalistic systems programming language.

## Project Goals
> Make the first mainstream systems langauge from Mexico.
> Explore the bare minimum of what a systems language must have to be usable, modern and ergonomic

## Features

- **12 keywords** — `if`, `loop`, `break`, `defer`, `ret`, `strun`, `fn`, `use`, `pub`, `pin`, `match`, `hot`
- **No hidden magic** — no GC, no type inference, no shadowing, no aliasing
- **Raw pointers** (`->`) and **fat pointers** (`=>`) with built-in slicing
- **Manual memory** — `alloc()` / `free()` with `defer` for cleanup
- **Packed structs** — no padding, predictable layout
- **C FFI** — `extern "C"` for direct interop
- **Rust-style errors** — colored diagnostics with source lines and carets
- **One-step compile** — `tcc source.tc -c app` transpiles and compiles in one command
- **Inline imports** — `@use "lib.tc"` inlines another `.tc` file at compile time
- **CLI args** — `i32 fn main: =>->i8 args { ... }` for command-line tools


## Philosophy

**Why it was made**: I wanted a simple systems language with less keywords than Go, without GC and without heavy runtime overhead.
**Backend**: For now it is C11 
**Concurrency and Safety?**: I've been toying around 

### Inspiration
- Pony
- Go
- C
- Rust

### Core principle:
> Everything that can be built with libraries has to be built with libraries. The core language is small.

## Quick Start

```bash
# Build the compiler
make

# Compile stdlib headers (only needed once)
./tcc stdlib/io.tc -o stdlib/io.h

# One-step: transpile + compile to binary
./tcc samples/fizzbuzz.tc -c fizzbuzz
./fizzbuzz

# Or two-step: transpile to C, then compile yourself
./tcc samples/fizzbuzz.tc -o fizzbuzz.c
gcc fizzbuzz.c -std=c11 -o fizzbuzz
```

## How It Works

Tight-C is a **source-to-source compiler** (transpiler) written in ~1800 lines of C. It reads `.tc` files and outputs portable C11.

```
source.tc → [Lexer] → [Parser] → [AST] → [Emitter] → output.c → gcc/clang → binary
```

### Pipeline

1. **Lexer** (`lexer.c`) — Tokenizes source into identifiers, literals, keywords, and symbols. Tracks line/col for error reporting.
2. **Parser** (`parser.c`) — Builds an AST from tokens. Handles operator precedence, scope-level `pin` enforcement, and `@use` file inlining (recursive parse + splice).
3. **Emitter** (`emitter.c`) — Walks the AST and outputs C11. Most constructs are 1:1 with targeted transforms:

| Tight-C | Emitted C |
|---------|-----------|
| `=>i32 s` | `tc_fat_i32 s` (struct with `.ptr` + `.len`) |
| `defer { free(p) }` | Scope-exit statements emitted in reverse order before `}` and `return` |
| `pin x` | Nothing emitted — enforced at parse time (compile error on reassignment) |
| `@use "lib.tc"` | Declarations inlined directly into AST (no `#include`) |
| `alloc(T, n)` | `TC_ALLOC(T, n)` → `calloc(n, sizeof(T))` |
| `=>->i8 args` in main | `main(int argc, char **argv)` + local fat pointer wrapping them |

### What it is *not*

There is no optimizer, no IR, no type inference pass, and no code generation beyond string concatenation of C. The output is always **readable, debuggable C** that you can inspect with `tcc source.tc -o source.c`.

## Design Goals

| Goal | How |
|------|-----|
| **Predictability** | Every line maps to obvious C. No hidden allocations, no implicit copies, no vtables. |
| **Simplicity** | 10 keywords. The entire compiler is a single-pass parser + tree-walk emitter. |
| **Portability** | Output is C11 with no platform-specific extensions. Compiles with gcc, clang, or any conforming C compiler. |
| **Safety without runtime cost** | Fat pointers carry length at zero overhead (struct field). `pin` catches mutation bugs at compile time. `defer` prevents resource leaks. |
| **Interop** | `extern "C"` blocks let you call any C library directly. `use` includes `.h` files. The generated code is linkable from C. |

### What Tight-C is good for

- **CLI tools** — parse args, process files, call system APIs
- **Embedded / bare-metal** — no runtime, no allocator required, predictable memory layout (packed structs)
- **Game engine internals** — manual memory, no GC pauses, direct pointer control
- **Learning compilers** — small enough to read in an afternoon, real enough to produce working binaries
- **C codebases that want better ergonomics** — fat pointers, defer, slicing, without leaving the C ecosystem

### What is intentionally skipped (for now)

- **Generics / templates** — explicitness over abstraction
- **OOP / inheritance** — composition via structs
- **Garbage collection** — manual memory is the point
- **Type inference** — all types are visible at declaration
- **Exceptions** — return error codes, check them
- **Runtime reflection** — if you need it, you're in the wrong language

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

### Control Flow
```
if (x > 0) { ... }

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
Type "tcc --error E000" for help

PS C:\Users\me\.projects\langs\tc> ./tcc --error E000
E000: Assignment to pinned variable

A variable marked with `pin` is immutable in the current scope.
You cannot reassign it with `=`, `+=`, `-=`, or any other assignment.

Bad:
    i32 x = 10
    pin x
    x = 11       // error: cannot assign to pinned variable

Fix: remove the `pin` or avoid reassigning the variable.
```
### Else if stmts
```
if(condition){
  // code
}
_if(condition){ 
  // code
}
_ { // "_" is the tC wildcard for otherwise/else
  // code
}

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

## Types

| Tight-C | C Equivalent |
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
tcc <input.tc> [-o output.c] [-c binary]
```

| Flag | Description |
|------|-------------|
| `-o file.c` | Emit transpiled C to file (`.h` gets `#pragma once`) |
| `-c binary` | Transpile + compile to binary (auto-detects gcc/clang) |
| (none) | Print transpiled C to stdout |

Combine both: `tcc app.tc -o app.c -c app` keeps the `.c` and builds the binary.

## Hot Reloading

Tight-C supports hot reloading of functions marked with the `hot` keyword. This allows you to modify code and recompile shared libraries without restarting the main executable.

### Basic Usage

```bash
# Initial compile with hot reload enabled
tcc hot.tc -H hotlib -c hotfn

# Run the application
./hotfn

# While running, modify hot.tc and rebuild only the hot library
tcc hot.tc -H hotlib --hot-rebuild
```

The running application will automatically detect the change and reload the library on the next hot function call.

### How It Works

Hot reload uses versioned shared libraries to avoid file locking issues on Windows:

- Functions marked with `hot` are compiled into a separate shared library
- Each rebuild creates a new version (e.g., `hotlib_1.dll`, `hotlib_2.dll`)
- A version file tracks the current version number
- The host executable monitors the version file and dynamically loads new versions

### Example

```tightc
use "stdlib/io.tc"

extern "C" {
    i32 fn Sleep: u32 ms {}
}

hot fn i32 add: i32 x, i32 y {
    ret x + y + 10
}

fn void main: {
    loop {
        i32 result = add(3, 4)
        printi(result)
        Sleep(2000)
    }
}
```

Running this prints `17` every 2 seconds. If you change `ret x + y + 10` to `ret x + y + 20` and rebuild with `--hot-rebuild`, the output will change to `24` without restarting the application.

### Additional Flags

| Flag | Description |
|------|-------------|
| `-H <libname>` | Enable hot reload mode, specify the library name |
| `--hot-rebuild` | Rebuild only the hot library (for running applications) |
| `-t, --temp` | Keep temporary .c files for debugging |

### Proof of Concept

See the `HOTSWAPPING/` folder for a complete working example with documentation, including the demo output showing hot reload in action.

This feature demonstrates Tight-C's capability for advanced systems programming patterns, using the industry-standard approach to hot reload on Windows (versioned libraries).

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

## Building the Compiler

Requires `gcc` (or `clang`) and `make`.

```bash
make          # Build tcc
make clean    # Remove build artifacts
```

---

<p align="center">
  <sub>Built by <a href="https://github.com/alonsovm44">@alonsovm44</a></sub>
</p>
