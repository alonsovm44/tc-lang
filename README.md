<p align="center">
  <h1 align="center">Tig 🦁</h1>
  <p align="center">Simplest possible, usable systems language</p>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/version-1.3.1s-blue?style=flat-square" alt="Version">
  <img src="https://img.shields.io/badge/language-C11-orange?style=flat-square" alt="Language">
  <img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" alt="License">
  <a href="https://ko-fi.com/alonsovm" target="_blank" rel="noopener noreferrer">
    <img src="https://img.shields.io/badge/Sponsor%20on-Ko--fi-29abe0?style=flat-square&logo=kofi&logoColor=white" alt="Sponsor on Ko-fi">
  </a>
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=flat-square" alt="Platform">
</p>

---
> Disclaimer: For stage0, Tig can use AI codegen for the C core (but test thoroughly), for stage1 of self hosting, handwritten code only. See [contributing](CONTRIBUTING.md) for more info.
> Note: see dogfood/ folder for some non trivial examples of Tig being used.

Tig is a minimalistic systems programming language. 🦁🦁🦁

## What's New in v1.3.2

1. Added C compiler flags using --
2. Added freestanding parameter to enable freestanding mode
3. Multiple file linking support
4. Added binary and hex literals
5. removed old function syntax
6. removed extern blocks
And more... check Changelog for full change reports.

## Project Goals
1. Make the first usable systems language from Mexico.
2. Explore the bare minimum of what a systems language must have to be usable, modern and ergonomic
3. Learn C and master Tig
4. Make an OS with it
5. Get a job

## Philosophy

**Why it was made**: I wanted an ergonomic systems language capable of anything, with less keywords than Go, no GC and without heavy runtime overhead; as safe as possible (but not necessarily proving safety). A language I could master and teach to others, a lang that puts Mexico in the map for once.

**Backend**: For now it is C11 
**Core Ideal**: Tig has to be able to fit in a single man's head. Small but powerful.

### Core principle:
> Everything that can be built with libraries has to be built with libraries. The core language is small.
> Explicit is better than implicit, but exceptions can be made...
> Flexibility over rigidity.
> Anything C can run, we run: Anywhere C has ran, we will run

## Features

- **18 keywords** — `if`, `loop`, `break`, `defer`, `ret`, `strun`, `fn`, `use`, `pin`, `match`, `else`, `enum`, `async`, `select`, `throw`, `try`, `catch`, `raw`.
- **Zero-Boilerplate Async** — Automatic runtime initialization, no manual setup needed
- **Async Functions** — Simple concurrent programming with `async fn`
- **Concurrent Data Structures** — Built-in `queue<T>` and `stack<T>` types
- **Ownership Transfer** — `@` operator for safe resource transfer
- **Select Statements** — Multi-operation waiting with `select`
- **Pin** — Keep variables alive across async boundaries
- **No hidden magic** — no GC, no type inference, no shadowing, no aliasing
- **Raw pointers** (`->`) and **fat pointers** (`=>`) with built-in slicing
- **Manual memory** — `alloc()` / `free()` with `defer` for cleanup
- **Packed structs** — no padding, predictable layout
- **Rust-style errors** — colored diagnostics with source lines and carets
- **One-step compile** — `tightc source.tc -c app` transpiles and compiles in one command
- **Inline imports** — `@use "lib.tc"` inlines another `.tc` file at compile time
- **CLI args** — `i32 fn main: =>->i8 args { ... }` for command-line tools


## Quick Start

Clone the repo and build the compiler:
```bash
# Build the compiler
make # requires make

# Compile stdlib headers in the stdlib folder (only needed once)
./tigc stdlib/io.tc -o stdlib/io.h # or you can do `@use "stdlib/io.tc"` to inline the lib and skip the .h file
# runtimes require the .h files (for now) since the runtime impl is in the .h file
# dont compile async.tc yet

# One-step: transpile + compile to binary
./tigc samples/fizzbuzz.tc -c fizzbuzz
./fizzbuzz
```

## How It Works

Tig is a source-to-source compiler (transpiler) written in ~4600 lines of C. It reads `.tc` files and outputs portable C11.

```
source.tc -> [Lexer] -> [Parser] -> [AST] -> [Checker] -> [Emitter] -> output.c -> gcc/clang -> binary
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

### What it is not
I don't claim Tig to be a memory safe language, it does not have a Borrow Checker or GC, rather memory management is manunal (although I plan to add arenas in the near future too), but it does provide some safety features like fat pointers, self cleaning error functions and defer statements as first class ergonomics to help write safer code.

Tig has no optimizer, no IR, no type inference pass, and no code generation beyond string concatenation of C. It can leverage LLVM flags from gcc/clang. The output is always readable, debuggable C that you can inspect with `tightc source.tc -o source.c`.

### What Tig is good for

- **CLI tools** , absolute tiny or no runtime overhead. Parse args, process files, call system APIs
- **Embedded / bare-metal** — no runtime, no allocator required, predictable memory layout with packed structs.
- **Game engine internals** — manual memory, no GC pauses, direct pointer control
- **Learning compilers** — small enough to read in an afternoon, real enough to produce working binaries
- **C codebases that want better ergonomics** — fat pointers, defer, slicing, without leaving the C ecosystem

## Hello World

```
use "stdlib/io.tc"

fn i32 main: {
    print("hello, world")
    ret 0
}
```

## Syntax Overview

### Variables
```
i32 x = 10
f64 pi = 3.14
u8 byte
```
Uninitialized variables default to `0`. The trade off is performance but we gain safety.
Variables are C-like and mutable by default. 
Declare constants with `pin`

```
f32 PI = 3.14; pin PI // immutable in the current scope

```

### Functions
```
fn i32 add: i32 a, i32 b {
    ret a + b
}
```

### Varargs
Functions can declare variadic arguments with `...`:
```
i32 fn printf: ->i8 fmt, ... {}

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
->i32 ptr = &x          // raw pointer (address-of)
->ptr = 99              // dereference

i32[4] arr = {1,2,3,4}
=>i32 slice = &arr       // fat pointer from array
printi(slice.len)        // built-in length
printi(slice.ptr[0])     // access elements

=>i32 sub = arr[1:3]     // slicing
```

### Pointer Combos
```
->->i32 pp = &p          // pointer to pointer
=>->i32 fps = &ptrs      // fat pointer of raw pointers, array of pointers
->=>i32 pslice = &slice  // raw pointer to fat pointer
=>=> sslice = &slice     // fat pointer to fat pointer
```
### Dereferencing

```tig
(->ptr) 

```

### Pointers to structs and pointer access
```
strun Point{
    i32 x,
    i32 y
}

fn void foo: ->Point p{
    p.>x = 10
    p.>y = 20 // .> is same as C's a->b

    //alternative
    (->p).x = 30
    (->p).y = 40 // same as C (*p).y 
}

```
Another example:
```
strun Point{
    i32 x,
    i32 y
}

Point p = {1, 2} // instanciation

fn void printP: ->Point p {
    printi(p.>x)
    printi(p.>y)

    // or this can be done too
    printi((->p).x)
    printi((->p).y)
}

```
Struns can have methods inside them

```
strun Point {
    i32 x,
    i32 y

    fn void printPoint: self {
        printf("[%d, %d]\n", self.x, self.y)
    }
}

fn void main:{
    Point p
    p.x = 10
    p.y = 20
    p.printPoint() // prints [10, 20]
}
```

### Control Flow
```
if (x > 0) { ... }
else if (x < 0) { ... }
else { ... }

loop { ... break }          // infinite loop unless break

loop if (i < 10) { ... }    // conditional loop, alias of while
```

### Memory
```
->i32 arr = alloc(i32, 100)
defer { free(arr) }
// or
defer free(arr) // braceless
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
"C"{
    #include "clib.h"
}

fn void wrapper:{
    // use C functions from "clib.h" here
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
Async functions are void always. Use channels (queues or stacks) to communicate between tasks.

```tig
use "stdlib/async.tc" // for the runtime
use "stdlib/snq.tc" // for stacks and queues types
use "stdlib/io.tc"

async fn void producer: queue<i32> q {
    q.push(100)
}

async fn void consumer: queue<i32> q {
    i32 value = q.pop()
    printi(value)
}

fn void main: {
    queue<i32> q = {}
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
        task1() => {
            printi("Task 1 completed")
        }
        task2() => {
            printi("Task 2 completed")
        }
    }
}
```

### Error system
Errors are a built in type in Tig via `error`. Errors are function like executable units, which contain how the error is handled plus clean up code.

Errors are declared with the `error` keyword, and can have parameters like functions, which carry the context of the error case. Errors are called with the `throw` keyword. 

#### Try-Catch blocks

Try-catch blocks are used to handle errors. They are declared with the `try` and `catch` keywords.
Catch blocks dont contain error handling code, rather they work like `match` statements where we specify which errors to catch and what return value to use. Use the `_` wildcard to catch all errors.

Example use of Tig's error system.
```tig
error zero_division: i32 a, i32 b{
    printf("Error, division by zero: %d / %d", a, b)
}

fn f64 divide: i32 a, i32 b{
    if(b == (f32)0){
        throw zero_division(a, b)
    }
    return (f64)a / (f64)b
}

fn i32 main: {
    i32 a = 10; i32 b = 0
    try{
        divide(a, b)
    }catch{
        zero_division(a, b) ret 1,
        _ { // wildcard, catches other exception
            printf("Unknown error")
            ret 1
        }
    }
    return 0
}

```
> Note: the error system might be tricky to use in freestanding environments due to no longjump and no debug printf statements available. Be sure to provide your own. 

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
tigc <input.tc> [-o output.c] [-c binary] [-t] -- [clang/gcc flags]
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

**File I/O**

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
| `sin`, `cos`, `tan`  | Trig functions          |
| `log`, `log2`, `log10` | Logarithms            |

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
