
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
Proposed change

<pub> fn <type> <name>: <type> <arg>, ... {

}

