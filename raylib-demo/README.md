# Raylib Demo in TightC

A simple bouncing ball demo using TightC and Raylib.

I know it is using inline C, and it is kinda like cheating, but this is just a PoC. 
## Prerequisites

- Raylib installed (https://github.com/raysan5/raylib/releases)
- MinGW-w64 or MSVC compiler

## Build Instructions

### Windows (MinGW)

```bash
# Transpile TightC to C
tigc main.tc -o main.c

# Compile with raylib
gcc main.c -o main.exe -I"C:/raylib/include" -L"C:/raylib/lib" -lraylib -lopengl32 -lgdi32 -lwinmm

# Run
./main.exe
```

### Windows (MSVC)

```bash
# Transpile TightC to C
tigc main.tc -o main.c

# Compile with raylib
cl main.c /Fe:main.exe /I"C:/raylib/include" /link /LIBPATH:"C:/raylib/lib" raylib.lib opengl32.lib gdi32.lib winmm.lib

# Run
main.exe
```

### Linux

```bash
# Transpile TightC to C
tigc main.tc -o main.c

# Compile with raylib
gcc main.c -o main -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Run
./main
```

### macOS

```bash
# Transpile TightC to C
tigc main.tc -o main.c

# Compile with raylib
gcc main.c -o main -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -lraylib

# Run
./main
```

## One-Step Build

If you have raylib in a standard location:

```bash
# Using tigc's integrated compilation (adjust raylib path as needed)
tigc main.tc -c main -I"C:/raylib/include" -L"C:/raylib/lib" -lraylib
```

## Features Demonstrated

- **Inline C**: Using `"C"{...}` blocks to call Raylib functions
- **Variable sharing**: TightC variables accessible in C blocks
- **Game loop**: Using TightC's `loop` keyword for the main loop
- **Type system**: Using `i32` for integers that map to C's `int32_t`

## Extending the Demo

Try adding:
- Multiple balls with different colors
- User input (mouse/keyboard) to control the ball
- Score tracking
- Particles when the ball bounces
 