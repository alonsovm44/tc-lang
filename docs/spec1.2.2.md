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

# macro
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
Useful for boilerplate
```
# main {
    hot pub fn void main: =>->args
}

main {
    //logic
}
```

Or if you have a very common long function signature

```
# fun01 {
    hot pub fn i32 
}

fun01 foo: int x{

}
fun01 bar: int y{

}
... // many functions with the same signature


```

## As a way to include files C style
```
# {"file/path.tc"}
```

Now we have three ways of including a file
```
use "path/to/file.tc" // expects a .h lib
@use "path/to/file.tc" inlines content to the AST, for when you want to include a .tc lib alone
# lib {"path/to/lib.tc"} pastes file content in the scope C style, can be used inside functions/methods/scopes
```

Use 
```
# math {"stdlib/math.tc"}
// can be used to call c libs (as long as they're single header)
```

Other way to use C files (.c and .h) is via "C".

```
{
    //global scope
    "C"{#include "libs/raylib.h"}
    // now we can use raylib functions
}

```

- Why three different ways?

| Feature | use | @use | # include |
|---------|-----|------|-----------|
| Level | Semantic (AST) | Semantic (AST) | Text (macro) |
| C header needed | Yes | No | No |
| Scope control | Global | Global | Anywhere |
| Type checking | Full | Full | Post-expansion |
| Use in functions | No | No | Yes |
| Best for | External libs | Internal libs | Templates/local utils |
Best for	External libs	Internal libs	Templates/local utils