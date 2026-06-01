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

# typedef 
Adding a word or something that renames types into aliases

```
type i32 = int 
//or
@i32 = int
//or 
#i32 = int 
```
