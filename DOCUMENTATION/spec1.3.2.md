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

## Function traits

Traits are compile-time attributes that modify function behavior. They are inspired by Rust's attribute system and provide a way to annotate functions with specific properties.

### Syntax

#### Single trait (inline syntax)
```tig
async fn void foo: {
    // async function
}
```

#### Multiple traits (block syntax)
```tig
#[async; naked; test]
fn void bar: {
    // async, naked, and test function
}
```

#### Deprecated but valid (inline for single trait)
```tig
async fn void baz: {
    // async code
}
```

**Note:** Use the block syntax `#[...]` when you have multiple traits to avoid overloading the function signature. Inline syntax is acceptable for single traits.

### Available traits

#### async
Marks the function as asynchronous, enabling async/await syntax and runtime support.
```tig
async fn void fetch_data: {
    // async code
}
```

#### test
Marks the function as a test case. Test functions are only compiled when running tests.
```tig
#[test]
fn void test_addition: {
    // test code
}
```

#### debug
Marks the function as debug-only. This function is removed in release builds.
```tig
#[debug]
fn void log_debug: str message {
    // debug logging code
}
```

#### volatile
Prevents the compiler from optimizing away the function, useful for hardware access.
```tig
#[volatile]
fn void read_hardware_register: u32 address -> u32 {
    // hardware register read
}
```

#### interrupt
Marks the function as an interrupt handler. The compiler generates appropriate prologue/epilogue for ISR context.
```tig
#[interrupt]
fn void timer_handler: {
    // interrupt service routine
}
```

#### naked
Generates a function without prologue/epilogue. Useful for low-level code like ISR stubs.
```tig
#[naked]
fn void isr_stub: {
    asm "iret"
}
```

#### nostd
Marks the function as not depending on the standard library. Required for freestanding/kernel code.
```tig
#[nostd]
fn void kernel_main: {
    // no libc dependencies
}
```

### Trait combinations

Traits can be combined to create specific function behaviors:

```tig
#[async; test]
fn void test_async_operation: {
    // async test function
}

#[interrupt; naked]
fn void isr_entry: {
    asm "jmp isr_handler"
}

#[volatile; nostd]
fn void direct_memory_write: u32 address, u32 value {
    // direct memory access in freestanding mode
}
```

### Implementation notes
- Traits are parsed at compile-time
- Multiple traits are separated by semicolons in the block syntax
- Traits can affect code generation, linking, and optimization
- Some traits are mutually exclusive (e.g., a function cannot be both `naked` and have standard prologue)

- NO custom traits for 1.3.2 patch

### Some traits don't make sense together:

1. async + naked → ❌ (async needs runtime, naked has no prologue)

2. interrupt + test → ❌ (ISRs aren't test functions)

3. volatile + debug → ❌ (debug implies logging, volatile is hardware)

The compiler should catch these combinations.

## Free standing mode

The compiler can now compile with no libc using --freestanding flag.

```bash
tigc file1.tc file2.tc -c app --freestanding
```

No stdio, no stdlib, no math, no setjmp.

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

#### 3. Interrupt handling attributes
```tig
// Mark functions as interrupt handlers
#[interrupt]
fn void timer_handler: {
    // ISR code
}

// Mark functions as naked (no prologue/epilogue)
#[naked]
fn void isr_stub: {
    asm "iret"
}
```

#### 4. Memory alignment
```tig
// Align variables to specific boundaries
align(16) u32 page_table[1024]
align(4096) u8 kernel_stack[8192]
```

#### 5. Direct memory access
```tig
// Access memory at specific addresses
extern "C" {
    u32 fn *0xB8000 video_memory
}
```

#### 6. Comptime if stmts
comptime if (condition){
  // this code gets into the final binary if the condition is true
}else if (condition){
  // this code gets into the final binary if the condition is true
}else{
  // this code gets into the final binary if the condition is true
}
```
Example
```tig
comptime if(OS == _WIN){
  // code that gets to the final binary if OS is windows
}else if (OS == _UNIX){
  // code that gets to the final binary if OS is unix
}
```

#### 7. No_stdlib attribute
```tig
// Mark entire modules as no_stdlib
#[nostd]
fn void kernel_main: {
    // No libc dependencies
}

```

#### 8. Linker script support
```bash
tigc kernel.tc -c kernel --freestanding --linker-script linker.ld
```

#### 9. Stack pointer manipulation
```tig
// Direct access to stack pointer
extern "C" {
    u8 fn get_stack_pointer: ->u8
    fn void set_stack_pointer: u8 sp
}
```

#### 10. Memory barriers
```tig
// Memory barrier for synchronization
memory_barrier()
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
1. Multiple file compilation with duplicate detection
2. -- flag for C compiler flags
3. Freestanding mode with full type definitions
3.1 comptime constants for windows and unix (needed to emulate #if _WIN macros on C)

MEDIUM PRIORITY:
4. Verbose output (-v)
5. Circular dependency detection

LOW PRIORITY:
6. Output directory (-o)
7. string type in stdlib

# Keywords for 1.3.2

- **18 keywords** — 
`if`, 
`loop`, 
`break`, 
`defer`, 
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
`comptime` (new) 
> Note: comptime was added for 1.3.2 OS type cases but not comptime{} blocks