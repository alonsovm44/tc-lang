# v 1.2.2 spec

## "For" loops
[DISCARDED] 
> It is unnecesary syntax sugar, postponed for later versions.
> Beware of code diabetes, it affects 3 out of 5 programmers (according to my butt, which is not a scientific source but still)
syntax sugar over for loops without `for` keyword
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
[DONE] 1 June 2026
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
# Simplifying hot reloading
Currently hot reloading is limited to hot functions. The process is verbose and tedious. It requires marking specific functions with a special syntax, which is an extra keyword.
The hot reloading feature is thought for the development process, when the finished product is done we just compile normally. All hot objects get their impl in the final bianry in production.

Idea: use hot compilation to move ALL the code into a dll/so

```bash
#Normal
./tigc source.tc -c app
#global hot
./tigc source.tc -c app -H hotlib
./app
#modify code while app is running
./tigc source.tc -H hotlib --hot 
#we shorten --hot-rebuild to just --hot
```
No need for hot keyword, ALL objects (struns, functions, enums, globals) are moved to the shared lib.
While more complex, this allows to hot modify all code, not just function impl.

# todo

- Automatic clean up of old dll/ hot versions. Only n and n-1 available for fallback


# keywords in 1.2.2

1. if 
2. loop
3. break
4. defer
5. ret
6. strun
7. fn
8. use
9. pub 
10. pin
11. match
12. else
13. enum 

Removed

`hot`
