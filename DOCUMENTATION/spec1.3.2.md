# 1.3.2 spec
CLI and Freestanding update

## New scheme for extern functions
[DONE]
Previously we used `extern "C"` for C interop. But it broke and became difficult to maintain. It also became redundant.
Old:
```
extern "C"{
    i32 fn printf: ->i8 fmt, ...
}

i32 fn myprintf: ->i8 fmt, ...{
    ret printf(->i8 fmt, ...)
}

```
This pattern proved to be more workable.
Inline C with `"C"` block including the library we want to port.
```
"C"{
    # include <stdio.h>
}
// then we define wrapper functions that call the C functions directly
fn void main: {
    printf("hello") // calling printf directly from C
}

fn i32 myprintf: ->i8 fmt, ...{ // defining a wrapper function
    ret printf(->i8 fmt, ...)
}

```
Thus extern "C" proved to be obsolete.

## Multiple comptime error prompting
[DONE]
Currently when Tig throws a comptime error it only shows one, when you fix it, it shows the next one, and so on. This is not ideal for debugging. The compiler should show all errors at once.

## Multiple file compilation
[DONE]
The compiler can now compile multiple files at once. The rules are:
  - Only one file can have main function, linker error otherwise
  - Error on duplicated symbols across files. (no namespaces, this is what collections will be for)
  - Other files are treated as libs
  - Compiler merges ASTs from all files
  - Circular deps trigger linker error
```bash
  tigc a.tc b.tc -c app
# Error: circular dependency detected: a.tc -> b.tc -> a.tc
```
  - main from whichever file has it becomes the entry point.

```bash 
tigc file1.tc file2.tc -c app 

```

## C compiler flags
[DONE]

The compiler can now pass flags to the C compiler using --

```bash
tigc file1.tc file2.tc -c app -- -O2 -lraylib -L/usr/lib
```
## C flags
The ability to pass compilation flags directly to the C compiler via the `--` separator.

### Basic syntax
```bash
tigc source.tc -c output [options] -- [C_compiler_flags]
```

The `--` separator marks where C compiler flags begin. Everything after it is passed directly to the C compiler.

### Optimization flags
```bash
# No optimization (faster compilation, larger binary)
tigc source.tc -c app -- -O0

# Level 1 optimization (balance)
tigc source.tc -c app -- -O1

# Level 2 optimization (recommended)
tigc source.tc -c app -- -O2

# Level 3 optimization (aggressive, slower compilation)
tigc source.tc -c app -- -O3

# Size optimization
tigc source.tc -c app -- -Os
```

### Debug and diagnostic flags
```bash
# Include debug symbols for GDB/LLDB
tigc source.tc -c app -- -g

# Include debug symbols with optimization (debug + optimized)
tigc source.tc -c app  -- -O2 -g

# Enable all warnings
tigc source.tc -c app -- -Wall

# Treat warnings as errors
tigc source.tc -c app -- -Werror

# Verbose compiler output
tigc source.tc -c app -- -v
```

### Performance and security
```bash
# Stack protection against buffer overflows
tigc source.tc -c app -- -fstack-protector

# Position independent code (ASLR support)
tigc source.tc -c app -- -fPIC

# Link-time optimization
tigc source.tc -c app -- -flto

# Disable frame pointer (extra register, performance gain)
tigc source.tc -c app -- -fomit-frame-pointer
```

### Platform-specific
```bash
# Target specific architecture (64-bit)
tigc source.tc -c app -- -m64

# Target 32-bit
tigc source.tc -c app -- -m32

# Enable SIMD instructions (AVX2)
tigc source.tc -c app -- -mavx2
```

### Multiple flags
```bash
# Combine multiple flags
tigc source.tc -c app -- -O3 -Wall -g

# Release build (optimized, no debug)
tigc source.tc -c app -- -O3 -Wall -DNDEBUG

# Debug build (symbols, warnings, no optimization)
tigc source.tc -c app -- -g -Wall -O0
```

### Preprocessing and macros
```bash
# Define a compile-time macro
tigc source.tc -c app -- -DMY_CONSTANT=42

# Multiple macros
tigc source.tc -c app -- -DDEBUG_MODE -DVERSION=\"1.0\"

# Include search paths
tigc source.tc -c app -- -I/usr/local/include -I./include
```

### Linking flags
```bash
# Link against a library
tigc source.tc -c app -- -lm  # Link math library

# Library search path
tigc source.tc -c app -- -L/usr/local/lib -lmylib

# Link static library
tigc source.tc -c app -- -static
```

### Practical examples
```bash
# Development: debugging with warnings
tigc samples/hello.tc -c hello -- -g -Wall -O0

# Release: optimized for speed
tigc samples/hello.tc -c hello -- -O3 -fomit-frame-pointer

# Hardened: security-focused
tigc samples/hello.tc -c hello -- -O2 -fstack-protector -Wall

# With external library (e.g., raylib)
tigc demos/raylib-demo/main.tc -c raylib_app -- -lraylib -L/usr/lib
```

## Free standing mode
[DONE]
The compiler can now compile with no libc using --freestanding flag.
This is meant to enable kernel development in Tig.
```bash
tigc file1.tc file2.tc -c app --freestanding
```

No stdio, no stdlib, no math, no setjmp.

This passes a --ffreestanding flag to clang/gcc/tcc/any other compiler used.
## Edge case:

```bash
tigc kernel.tc -c kernel --freestanding -- --ffreestanding # using --ffreestanding flag and --freestanding flag, this will pass --ffreestanding twice to the compiler, so the compiler ignores one of them. 
```
### Minimal type definitions provided in freestanding mode:

```c
// Fixed-width integer types
typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;
typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;

// Pointer types
typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;

// Size types
typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

// NULL definition
#define NULL ((void*)0)

// Volatile for hardware register access
#define volatile __volatile__
```
## Constants
[ENFORCED]
So far `pin` enforces immutability in the current scope. I considered adding `const` but it would require more complex type checking and I think it's not necessary for now.

If you want global constants use macros
```
# PI {3.14159}
# PORT {0x60}
# SCREEN_WIDTH {680}
# SCREEN_HEIGHT {480}

fn i32 main: {
    println("PI: {}", PI);
    println("PORT: {}", PORT);
    println("SCREEN_WIDTH: {}", SCREEN_WIDTH);
    println("SCREEN_HEIGHT: {}", SCREEN_HEIGHT);
    return 0;
}
```
## Global variables

```
i32 x = 10
strun Point{
    i32 x,
    i32 y
}

.boot Point point = {10, 20}

fn i32 main: {
    // code
}

```

## OSdev keywords

### Raw (volatile)
[DONE]
For me `raw` is a better term than volatile. 
```
raw fn some_function: {
    // function that accesses memory directly
}

raw fn void write_vga: u16 offset, u8 value {
    ->u8 vga = (->u8)0xB8000
    vga[offset] = value
}
```

### New keywords

- raw (aka volatile) [DONE]
    - Marks variable or function as raw (compiler should not optimize accesses)

## Inline assembly

asm fn foo: i32 a, i32 b{
    mov rax, 3
    mov rbx, 5
    add rax, rbx

    outl %a, %b
}

## 2. Memory section types
[DONE]
```tig
// Place data in specific sections
.data u32 kernel_data = 0x1000
.bss u32 zero_data
.rodata u8 const_data = 42
```
## memory section definition
You can define custom memory section with the .NAME syntax

.mysection u8 mydata = 42

This transpiles to:
```c
__attribute__((section(".mysection"))) uint8_t mydata = 42;
```

Example uses

.rodata raw fn void myrodata_function: {
    // function that accesses memory directly
}

.mysection fn i32 add: i32 a, i32 b{
    ret a + b
}

.rodata strun Point{
    i32 x, i32 y
}


### 2.1 Hex and binary literals
[DONE]
0x00001
0b10101 // assignable to variables
0xFF // assignable to variables

#### 3. Memory alignment
```tig
// Align variables to specific boundaries
align(16) u32[1024] page_table
align(4096) u8[8192] kernel_stack
```

#### 4. Direct memory access
```tig
// Access memory at specific addresses
// (a value between brackets is interpreted as a memory address)
u32 fn [->0xB8000] video_memory{
    // access video memory at 0xB8000
}
```

## Compile time if stmts (macros)
We already have comptime macros

```
# mymacro (T,a,b){
    // code that gets to the final binary
    //evalutated at compile time
    T a = b + b
}

```
Special case macros if, else if and else
```
# If (condition){
  // this code gets into the final binary if the condition is true
}
# Elseif (condition){
  // this code gets into the final binary if the condition is true
}
# Else{
  // this code gets into the final binary if the condition is true
}
```
Example:
```tig
# If(_WIN){
  // code that gets to the final binary if OS is windows
}
# Elseif(_UNIX){
  // code that gets to the final binary if OS is unix
}
# Else{
  // other OS
}
```
More in depth comptime features deferred for 1.3.3


# Keywords for 1.3.2

- **18 keywords** — 
`if`, 
`loop`, 
`break`, 
`defer`, s
`ret`, 
`strun`, 
`fn`, 
`use`, 
`pin`, 
`match`, 
`else`, 
`enum`, 
`async`, 
`select`,
`throw`,
`try`,
`catch`,
`raw`, (new) (it is same as volatile)

## New types

`.rodata`
`.bss`
`.data`

## New reserved macros

```
# If{}
# Elseif{}
# Else{}
_WIN
_UNIX
```