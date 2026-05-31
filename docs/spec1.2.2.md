# v 1.2.2 spec

## For loops

syntax sugar over for loops without for keyword

```
loop variable, range..top{
    // logic
}

Example

loop i, 0..10{

}
loop k, 10..0{

}

loop j, 0..a{

}

loop i, a..b{

}

loop i, b..0{
    
}
```

# Sizeof and co.
Adding built in functions for C parity
1. sizeof()
2. typeof()
3. alignof()
4. offsetof()

The rest can be built in library