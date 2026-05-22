# Tight-C

Simplest possible, usable systems language

The goal is to create a language that is as simple as possible while still being useful. Everything that can be built in the stdlib has to.

Rules:
    - No GC
    - No safety, but defer included
    - No inference, everything is explicit
    - No shadowing, pick names well
    - No OOP, imperative procedural
    - No indentation, braces
    - No aliasings to avoid bugs
    - No string type, C like strings
    - Math built in
    and: &, or: |, xor: ^, not: !, shl: <<, shr: >>
    - + - * / %
    - eq == ne <> lt < gt > le <= ge >=
    - Incrementals and decrementals
        - ++, --
        - +=, -=, *=, /=, %=

    - Uninitalized variables default to 0
    - Uninitalized arrays default to 0
    - Manual scopes
    - Functions return only one value

    Overflow semantics:
    1111 << 1 = 1110 (wraps)

# Features for v1.0.0 only
    1. Variables
    2. Functions
    3. Structs
    4. Loops + break
    5. Raw pointers (->)
    6. Fat pointers (=>)
    7. Fixed size arrays
    8. Manual allocation
    9. defer
    10. C FFI 

# comments
// inline
/**/ multiline

# scopes
Manual scoping.

Begin scope { 

End scope }

{
 // scope 1
    {
        //scope 1.1
    }
    {
        // scope 1.2
    }

}
{
        // scope 2
    }

Scope 1.1 knows what is in scope 1 but not in scope 1.2 or 2

# // TYPES
void 
i2, i4, i8, i16, i32, i64   // signed
u2, u4, u8, u16, u32, u64   // unsigned
f32, f64            // floats

# // VARIABLES (default 0)
<type> <name> = <value>
u8 x = 10
i32 y

## pinning
Pinning makes a variable immutable in the current scope

pin <var>


# // FUNCTIONS
<type> fn <name>: <type> <arg>, ... {
    ret <value>
}
void fn main: {
    ret
}

# // STRUCTS (packed)
struct <name> {
    <type> <name>
}
struct Point { i32 x, i32 y }
Point point // instances of structs default to 0

point.x = 10.2


# // POINTERS
## Raw pointers
-><type>         // pointer type
@<var>          // address-of
-><ptr>         // dereference

i32 x = 10
->i32 ptr = @x 
-->i32 ptr2 = @ptr

## Strings

// String literals (decay to ->u8)
->u8 hello = "hello"     // pointer to first character

// Dereference to get character
u8 first = ->hello       // 'h'

// Index for convenience
u8 second = hello[1]     // 'e'

// Null terminator check
loop {
    if (hello[i] == 0) { break }
    // process character
    i = i + 1
}

# Fat pointers
Fat pointers are always a slice.
=><type> <name> = arr[<start>:<end>]

Example:

i32[7] arr = {0,1,2,3,4,5,6}

=>i32 slice = arr[1:4]

i32 x = lenof(slice) // equals 4

->u8 str = slice // equals "123"

->u8 first = slice[0] // equals '1'

## strings with fat pointers

=>u8 hello = "hello" // fat pointer with length

u8 size = lenof(hello) // equals 5

u8 first = hello[0] // equals 'h'

u8 last = hello[size - 1] // equals 'o'

## Fat pointers in functions

void fn foo: =>i32 slice {
    // slice is a fat pointer
    

}

# // ARRAYS (fixed size)

<type>[<size>] <name>
i32[5] arr = {0,1,2,3,4}

# // BUILTINS

sizeof(<type>)   // bytes
lenof(<array>)   // element count
cast(<expr>, <type>)

# // MEMORY

alloc(<type>, <count>)
free(<ptr>)

# // CONTROL

if (<cond>) { ... }

loop { ... break }

loop if (<cond>) { ... }

# // OPERATORS

& | ^ ! << >>     // bitwise
+ - * / %         // arithmetic
== <> < > <= >=   // comparison
&& ||            // logical

# Defer

defer {
  // code to execute at the end of the scope
}

Example:

->i32 arr = alloc(i32, 100)
defer { free(arr) }
# FFI

extern "C"{
    void fn printf(->i8 format, ...)
}

# Modularity
use "path/to/life"

All objects are private by default, to use them they have to be declared as public

pub i32 fn add: i32 a, i32 b {
    ret a + b
}

pub struct Point {
    i32 x,
    i32 y
}

pub i32 x = 10 


# Keywords

1. if 
2. loop
3. break
4. defer
5. ret
6. struct
7. fn
8. use
9. pub 
10. pin
 