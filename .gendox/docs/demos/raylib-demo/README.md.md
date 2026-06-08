<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: 1b9dd4128bfda2ff
timestamp: 2026-06-07T22:44:45Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# Raylib Demo in TightC

A simple bouncing ball demo using TightC and Raylib. This is a proof of concept demonstration where inline C syntax is used for integration with Raylib, which could be considered unconventional but serves as an educational example.

## Prerequisites

To run this demo successfully, you need to have the following prerequisites:

- **Raylib**: A cross-platform game development library that supports Windows, Linux, macOS, and more. You can download it from [this GitHub release page](https://github.com/raysan5/raylib/releases).
- **MinGW-w64** or **MSVC Compiler**: A compiler environment is required to build the application.

## Build Instructions

Depending on your operating system (Windows, Linux, macOS), you have different ways to compile this demo. Here are instructions for each platform:

### Windows (MinGW)

1. Convert the TightC source code to C using TIGC (Tight Inline Code Generation):
   ```bash
   tigc main.tc -o main.c
   ```
2. Compile with Raylib and link against its libraries:
   ```bash
   gcc main.c -o main.exe -I"C:/raylib/include" -L"C:/raylib/lib" -lraylib -lopengl32 -lgdi32 -lwinmm
   ```
3. Run the compiled executable:
   ```bash
   ./main.exe
   ```

### Windows (MSVC)

1. Convert the TightC source code to C using TIGC:
   ```bash
   tigc main.tc -o main.c
   ```
2. Compile with Raylib and link against its libraries:
   ```bash
   cl main.c /Fe:main.exe /I"C:/raylib/include" /link /LIBPATH:"C:/raylib/lib" raylib.lib opengl32.lib gdi32.lib winmm.lib
   ```
3. Run the compiled executable:
   ```bash
   main.exe
   ```

### Linux

1. Convert the TightC source code to C using TIGC:
   ```bash
   tigc main.tc -o main.c
   ```
2. Compile with Raylib and link against its libraries:
   ```bash
   gcc main.c -o main -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
   ```
3. Run the compiled executable:
   ```bash
   ./main
   ```

### macOS

1. Convert the TightC source code to C using TIGC:
   ```bash
   tigc main.tc -o main.c
   ```
2. Compile with Raylib and link against its libraries:
   ```bash
   gcc main.c -o main -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL -lraylib
   ```
3. Run the compiled executable:
   ```bash
   ./main
   ```

### One-Step Build

If you have Raylib installed in a standard location, TIGC can compile the code directly and link against the necessary libraries:

```bash
tigc main.tc -c main -I"C:/raylib/include" -L"C:/raylib/lib" -lraylib
```

This command will create an executable named `main` that integrates with Raylib.

## Features Demonstrated

### Inline C

The demo uses inline C code blocks enclosed within `"C"{...}`. This approach allows for easy integration between TightC and Raylib, providing a way to leverage native C syntax for interacting with Raylib functions such as drawing shapes, handling input, etc.

### Variable Sharing

TightC variables can be accessed within the C blocks of the demo. These variables are declared in main.tcc file which is compiled by TIGC before being included in `main.c`. This shared state allows for easy synchronization between the control flow logic written in TightC and the graphical rendering code written in C.

### Game Loop

The demo uses a tightc’s `loop` keyword to implement a game loop. The function `main_loop` is decorated with `@tightc.loop` which automatically calls it repeatedly, facilitating seamless integration with the main logic of your application.

### Type System

TightC supports a type system that maps directly to C types like `i32` for integers (equivalent to `int32_t`), ensuring compatibility and performance. This feature simplifies the integration between TightC and Raylib, making it easier to work with both languages together seamlessly.

## Extending the Demo

To extend this demo further, you can explore adding more features such as:

- **Multiple Balls**: Create multiple instances of a ball object and control their positions or animations.
- **User Input Handling**: Implement controls for mouse clicks (for collision detection) and keyboard input to move the balls around.
- **Score Tracking**: Introduce mechanisms to keep track of scores based on collisions with walls, other objects, etc.
- **Particles Effects**: Add visual effects like particle explosions when a ball bounces off something or collides.

These additions would enhance the complexity and interactivity of your game, making it more dynamic and engaging for users.