# v1.3.1

## Try/Catch blocks for error handling
Simple try/catch blocks for error handling, no complex error types or traits.
```tc
try {
    // Code that might throw
    int result = divide(10, 0);
} catch (error) {
    // Handle error
    print("Error: ", error);
}
```

## Comptime execution
Following the "explicit" nature of the language, comptime execution is explicit and requires the `comptime` keyword.
```tc
comptime {
    // Code that executes at compile time
}
```

Example
```tc
fn void make_array(int size) {
    comptime {
        "C"{
            // Code that executes at compile time
            int[] arr = new int[size];
            for (int i = 0; i < size; i++) {
                arr[i] = i;
            }
        }
    }
}
// or this

comptime {
    // Imagine this reads a config file or a JSON string
    ->u8 data = "10,20,30,40"
    
    // Generate a C array initialization
    "C"{
        int lookup_table[] = {%s};
    }
}

fn void main: {
    // lookup_table is now a real C array in your binary
    printi(lookup_table[1]) // Prints 20
}

// other example

fn void generate_adders: {
    i32 i = 0
    loop if(i<=100){
        // generate functions at comptime
        "C"{
            int add_%d(int a, int b) {
                return a + b + %d;
            }
        }
        i++
    }
}

comptime {
    generate_adders()
}

fn void main:{
    printi(add_99(1, 2)) // Prints 102
}
```

## Conditional defer
We tried to introduce this before but it didnt work and was buggy. While we could just do this

if(cond){
    defer{}
}else
{

}

we could also have
```
defer if(cond){

}

```

## FFI generator
Foreign Function Interface Generator Parse C headers and generate Tig bindings automatically:
```tc
extern "C" @port "raylib.h"  // Auto-generates all bindings
// Now you can use Raylib functions directly
```
Why it blows minds: No more manually writing extern declarations. Instant access to any C library.
```tc
// Wrapping in comptime ensures the generator runs at compile time
comptime{
    extern "C" @port "raylib.h"
}

```
This is basically a macro system or #include for any C library, but it's more powerful because it can generate code at compile time.

Example conditional binding generator

```
if (use_raylib) {
    comptime {
        extern "C" @port "raylib.h"
    }
}else if(use_sdl) {
    comptime {
        extern "C" @port "SDL2/SDL.h"
    }
}

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
tigc source.tc -c app -O2 -- -g

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

## Stack and Queue update
Currently we have basic stacks and queue implementation

To be added:

- Fixed size stacks and queues
```
// example
stack<i32> s[10] = {}
queue<f32> q[5] = {} // a queue of 5 floats

// multiple types
stack<i32, f32, char> s2[10] = {} // this stack accepts integers, floats and characters only

// wildcard

stack<*> s3[10] = {} // this stack accepts any type

// stack of stacks
stack<stack<i32>> s4[5] = {} // this stack accepts stacks of integers only

Same applies to queues.
```

Bad:
```tig
stack<*, i32> s = {} // this is not allowed, since * already includes i32
```
