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