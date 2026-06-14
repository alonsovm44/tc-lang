# Spec list for version 1.1.0

# function pointers
[done]
Useful for callbacks

Normal pointers

i32 x = 12
->i32 p = @x

## Function pointers

i32 fn add: i32 a, i32 b {
    return a + b
}

void fn main: {
    fn(i32, i32)i32 fptr = @add
}

## Passing it to a function

i32 fn add(i32 a, i32 b) {
    return a + b
}


fn i32 callback: fn(i32, i32)i32 fptr, i32 a, i32 b {
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
[DONE] May 26
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
[DONE] date: 25 may 2026
As commented by procedural
> "Also, C allows not only dereferencing raw pointers, but treat them as arrays, for example "p->x" works, but "p[0].x" also works. This is useful for accessing array elements of a foreign pointer"

Tight-C already supports treating raw pointers as arrays, you just index them directly with ptr[i]. The -> prefix is only for single-element dereference (->ptr = C's *ptr), while ptr[i] = C's ptr[i].


# function syntax fix
[DONE] date today: May 24 2026
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

# strunions
[DONE]
Both a struct and a union this way we dont need two keywords for structs and unions
Now we have a spectrum

[struct]—[strun]—[union]

strun MyStrun{

    i32 field1
    i32 field2
    // field1 and field2 are 4 bytes and 4 bytes each
    &i32 field3
    &f32 field4
    // field 3 and field 4 share the same adress
}

A normal struct is just a strun without &'s
A union is a strun with all &'s

Example struct

strun Point{
    
    i32 x
    i32 y
}

Example strun

strun hybrid{
    i32 x
    i32 y
    &i32 z
    &f32 w
}

Example union

strun Data{
    &i32 data
    &str ip
}   

# memory grouping in struns
[DONE] May 25
Currently 1.1.0 struns group memory in the same adress unless spacing is found

Currently
strun Data{
    &i32 x
    &i32 y
    i32 w
    ...
    &f32 z
    &f64 a
    ...
}

x and y live in the same adress, then w appears and adds 4 bytes, then z share the same adress. 
In order to save memory we can do this

strun Data{
    &i32 x
    &i32 y
    i32 // anonymous memory, it separates the first adress by 4 bytes
    &f32 z
    &f64 a
    u8 // anonymous memory, it separates the second block by 1 byte
    &str c
    &i8 d
}

# non packed structs, and new rules
[DONE]
As of 1.0.0 and 1.1.0, struns are packed by default. Which i implemented for no known reason. I think we could replacea this, not by adding a `packed` keyword but a symbol

New rule:
    - Struns are not packed by default
    - to pack a strun use @
    
@strun Data{
    // packed data
}

# implicit else
## idea 3
[SELECTED] [DONE] May 28
if(condition){

}
_if(condition){

}
_if(condition){

}
_ { // else

}
This fits tC semantics best

# Keywords in 1.1

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
11. match (new)
