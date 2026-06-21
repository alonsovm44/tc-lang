# 1.3.2 spec
CLI and Freestanding update

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

## OSdev keywords
### Raw (volatile)
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

### interrupt
Marks the function as an interrupt handler. The compiler generates appropriate prologue/epilogue for ISR context.
```tig
interrupt fn void timer_handler: {
    // interrupt service routine
}
```

### naked
No prologue/epilogue generated by the compiler. Useful for assembly stubs.
```tig
naked fn void isr_stub: {
    asm "iret"
}
```

### freestanding
Marks the function as not depending on the standard library of Tig or libc. Required for freestanding/kernel code.
```tig

freestanding fn void kernel_main: {
    // no libc dependencies
}
```
### New keywords

- raw (aka volatile)
    - Marks variable or function as raw (compiler should not optimize accesses)
- naked
    - Marks function as having no prologue/epilogue generated by the compiler
- interrupt
    - Marks function as an interrupt handler
- freestanding (same as nostd)
    - Marks function as non dependent on the stdlib or any particular OS
```
freestanding volatile fn void write_port: u16 port, u8 value {
    // write to hardware port
}
```

### Some traits don't make sense together:

- async + naked → ❌ (async needs runtime, naked has no prologue)

The compiler should catch these combinations.


## Inline assembly

"ASM"{
    // Inline assembly block
    mov eax, 1
    mov ebx, 2
    add eax, ebx
}


### Critical kernel development features:

#### 1. Inline assembly support
```tig
fn void write_port: u16 port, u8 value {
    asm volatile "outb %0, %1" (value, port)
}
```

#### 2. Memory section attributes
```tig
// Place data in specific sections
pin .data u32 kernel_data = 0x1000
pin .bss u32 zero_data
pin .rodata u8 const_data = 42
```

### 2.1 Hex and binary values

0x00001
0b10101 // assignable to variables

#### 3. Memory alignment
```tig
// Align variables to specific boundaries
align(16) u32 page_table[1024]
align(4096) u8 kernel_stack[8192]
```

#### 4. Direct memory access
```tig
// Access memory at specific addresses
// (a value between brackets is interpreted as a memory address)
extern "C" {
    u32 fn [->0xB8000] video_memory{
        // access video memory at 0xB8000
    }
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
# IF (condition){
  // this code gets into the final binary if the condition is true
}
# ELSEIF (condition){
  // this code gets into the final binary if the condition is true
}
# ELSE{
  // this code gets into the final binary if the condition is true
}
```
Example:
```tig
# IF(_WIN){
  // code that gets to the final binary if OS is windows
}
# ELSEIF (_UNIX){
  // code that gets to the final binary if OS is unix
}
# ELSE{
    // other OS
}
```
More in depth comptime features deferred for 1.3.3

## 7. Linker script support
```bash
tigc kernel.tc -c kernel --freestanding --linker-script linker.ld
```

#### 8. Stack pointer manipulation
```tig
// Direct access to stack pointer
extern "C" {
    u8 fn get_stack_pointer: ->u8 sp
    void fn set_stack_pointer: u8 sp
}
```

#### 9. Memory barriers
```tig
// Memory barrier for synchronization
void fn memory_barrier: ()
```

### Implementation notes:
- Change in emitter.c: Add a freestanding flag to emit_program() that skips all libc includes and defines minimal types.
- Add inline assembly parser support
- Add section attribute parser
- Add interrupt/naked attribute support
- Add alignment attribute support
- Add comptime OS detection constants

# priorities
HIGH PRIORITY:
1. Multiple file compilation with duplicate detection [DONE]
2. -- flag for C compiler flags [DONE]
3. Freestanding mode with full type definitions 
3.1 comptime constants for windows and unix (needed to emulate #if _WIN macros on C)

MEDIUM PRIORITY:
4. Verbose output (-v)
5. Circular dependency detection

LOW PRIORITY:
6. Output directory (-o)
7. string type in stdlib

# Keywords for 1.3.2

- **22 keywords** — 
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
`raw`, (new)
`freestanding`, (new)
`naked`, (new)
`interrupt`, (new)
