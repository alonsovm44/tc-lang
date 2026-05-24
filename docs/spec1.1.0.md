
# function pointers
Useful for callbacks

Normal pointers

i32 x = 12
->i32 p = @x

## Function pointers

i32 fn add(i32 a, i32 b) {
    return a + b
}

void fn main: {
    ->(i32 fn(i32, i32)) fptr = @add
}

## Passing it to a function

i32 fn callback: ->(i32 fn(i32, i32)) fptr, i32 a, i32 b {
    return fptr(a, b)
}

void fn main:{
    i32 result = callback(add, 5, 3)
    print(result)
}

# Inlining .tc files at compile time
[DONE]
Some .tc files might not need a .h file counterpart

For this we specify with @

@use "lib.tc" 

this inlines a .tc file declarations at compile time, no .h file needed.

# match statement

match(value){
    0 = {
        print("zero")
    }
    1 = {
        print("one")
    }
    _ = {
        print("other")
    }
}
# pointers to structs
[DONE]
struct Node{
    i32 data
    ->Node next
}

# Pointers as arrays
As commented by procedural
> "Also, C allows not only dereferencing raw pointers, but treat them as arrays, for example "p->x" works, but "p[0].x" also works. This is useful for accessing array elements of a foreign pointer"

Tight-C already supports treating raw pointers as arrays, you just index them directly with ptr[i]. The -> prefix is only for single-element dereference (->ptr = C's *ptr), while ptr[i] = C's ptr[i].


# function syntax fix

Someone noted on hacker news that function syntax is verbose, 

Proposed for 1.1
<pub> fn <type> <name>: <type> <arg>, ... {

}

Example:

fn void main: {
    // code
}

pub fn i32 add: i32 a, i32 b {
    ret a + b
}

fn ->Point foo:  ->foo bar {
    // code
}

# Better error reporting
[DONE]
For version 1.0.0 we had rust like error reporting in the CLI, for version 1.1 we are adding version clalssification spec file
and built in explainations in the compiler. Rust styled.

# functions can return tuples

Useful for error reporting and very Go-like
    
fn (typea, typeb) foo: {

}

example
fn (i32, ->i8) foo: {
    (i32, ->i8) x = (12, "shazam!")
    ret x
}