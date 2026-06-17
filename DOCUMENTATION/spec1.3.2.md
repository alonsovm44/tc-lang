# 1.3.2 spec
CLI update

## Multiple file compilation

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

The compiler can now compile with no libc using --freestanding flag.

```bash
tigc file1.tc file2.tc -c app --freestanding
```

No stdio, no stdlib, no math, no setjmp
Just these:
```c
typedef __SIZE_TYPE__ size_t;
typedef __UINTPTR_TYPE__ uintptr_t;
```
Or define your own fixed-width types
Change in emitter.c: Add a freestanding flag to emit_program() that skips all libc includes and defines minimal types.

# priorities
HIGH PRIORITY:
1. Multiple file compilation with duplicate detection
2. -- flag for C compiler flags
3. Freestanding mode with full type definitions

MEDIUM PRIORITY:
4. Verbose output (-v)
5. Circular dependency detection

LOW PRIORITY:
6. Output directory (-o)
7. string type in stdlib

# Keywords for 1.3.2

- **17 keywords** — 
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