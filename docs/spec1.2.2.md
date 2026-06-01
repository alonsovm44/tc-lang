# v 1.2.2 spec

## "For" loops

syntax sugar over for loops without for keyword
The variable is automatically declared as i32 and set to the first value of the range
(in this case we make an exception for no shadowing, loop variables can be shadowed)
```
loop variable, range..top{
    // logic
}

Example

loop i, 0..10{

}
loop k, 10..0{

}

// limits checked in runtime:
loop j, 0..a{

}

loop i, a..b{

}

loop i, b..0{
    // if b is negative the loop is incremental
    // if b is positive the loop is decremental
}
```

# Sizeof and co.
Adding built in functions for C parity
1. sizeof() [DONE] 31 March 2026
2. typeof()
3. alignof()
4. offsetof()

The rest can be built in library



# macros
Text replacement macros
Allows to customize the language

```
# name {target}
// example

# int {i32}
# float {f32}
# func {fn}

func void main: {} // now we can use func instead of fn 

// other example

# printHi {print("Hello World!")}
// now we can use printHi instead of print("Hello World!")

func void foo:{
    printHi
}
```
## Other uses
Renaming the macro

```
# macro{#}
macro printHi {print("Hello World!")}

```
- Useful for boilerplate
```
# main {
    hot pub fn void main: =>->args
}

main {
    //logic
}

# check_error{
    if(status != 0){
        // error handling
        print("error detected")
        ret 1
    }
}

fn void foo: {
    // code
    check_error // this expands on comptime
}
```

- Or if you have a very common long function signature

```
# hpfint {
    hot pub fn i32 
}

hpfint voidfoo: int x{

}
hpfint voidbar: int y{

}
... // many functions with the same signature
```

Other way to use C files (.c and .h) is via "C".

```
{
    //global scope
    "C"{#include "libs/raylib.h"}
    // now we can use raylib functions
}

```

## Macros are scoped
```

//global scope
# float {f32} // f32 is float in all scopes
// not recommended since it can collide with other macros
    {
       # int32{i32}
    } // int32 dies here
    {
        # int {i32}
    } // int dies here


fn void foo: {
    # int{i32}
    // i32 is int here
}// after this int dies
```

## Parametric macros

```
# name(params,...){
    //body
}
// Examples

# swap(T, x, y){
    T temp = x
    x = y
    y = temp
}

fn void main: {
    i32 a = 1
    i32 b = 2
    swap(i32, a, b)
    printi(a)
    printi(b)

    // works for floats too
    f32 c = 1.5
    f32 d = 2.5
    swap(f32, c, d)
    printf(c)
    printf(d)
}
```
