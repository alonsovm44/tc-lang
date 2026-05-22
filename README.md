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

Tight-C is a minimal systems programming language that transpiles to C.
**10 keywords**, no garbage collector, no inference, no OOP — just explicit, predictable code with C-level power.

## Features

- **10 keywords** — `if`, `loop`, `break`, `defer`, `ret`, `struct`, `fn`, `use`, `pub`, `pin`
- **No hidden magic** — no GC, no type inference, no shadowing, no aliasing
- **Raw pointers** (`->`) and **fat pointers** (`=>`) with built-in slicing
- **Manual memory** — `alloc()` / `free()` with `defer` for cleanup
- **Packed structs** — no padding, predictable layout
- **C FFI** — `extern "C"` for direct interop
- **Rust-style errors** — colored diagnostics with source lines and carets
- **One-step compile** — `tcc source.tc -c app` transpiles and compiles in one command
- **Inline imports** — `@use "lib.tc"` inlines another `.tc` file at compile time
- **CLI args** — `i32 fn main: =>->i8 args { ... }` for command-line tools

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

## Hello World

```
use "stdlib/io.tc"

void fn main: {
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
i32 fn add: i32 a, i32 b {
    ret a + b
}
```

### Structs
```
struct Point {
    i32 x,
    i32 y
}

Point p
p.x = 10
```

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
```

### Control Flow
```
if (x > 0) { ... }

loop { ... break }          // infinite loop

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
error: bare identifier 'w' is not a statement
 --> samples/syntaxe.tc:7:9
   |
 7 |         w
   |         ^ bare identifier 'w' is not a statement
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

## Philosophy

> Everything that can be built in the stdlib has to.

Tight-C bets that C's power doesn't require C's complexity. Strip away the historical baggage and you get a language a single person can implement, understand fully, and still write real systems code in.

---

<p align="center">
  <sub>Built by <a href="https://github.com/alonsovm44">@alonsovm44</a></sub>
</p>
