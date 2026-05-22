
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

