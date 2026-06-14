# v1.3.1
This patch introduces an intuitive error system. I tried to make something that is easy to learn and intuitive to use.
[DONE] partially
<!-- TODO: 
1. Fix if statements not working in try blocks
-->
## Error system
[DONE]
[STILL BUGGY]
Add an error type 

error MyError: T args,... {
    // code to be executed on error time
}

Example:
error BufferOverflow: i32 size, ->buffer buf{
    // code that executes when the exception happens
    printf("Error buffer overflow in buffer %s with size %d", buf, size)
    free(buf)
}

fn void myFunction(): {
    // function body
    try {
        risky_fn()
    }
    catch {
        BufferOverflow(1024, buf) ret 1,
        myErrorFoo(a,c,d) ret 0,
        myError2(d,e,f) ret -1, 
        _ {
            // code that executes when any other error is thrown
            ret 1
        }
        // wildcard, matches any error
        //catch has no body, we just list the error "functions" that could be thrown, if none are thrown, the catch block is not executed
    }   // the `ret X` part is optional, if not specified, the default return value is 1
}
### Other rules
- The `ret` statement is optional, if not specified, the default return value is 1
- You can't throw errors inside error
- Void functions dont return 1 as a default, it is the only exception.
Bad:

    error MyError: {
        throw OtherError(arg) // not allowed
    }
### Practical example
```
// Define errors with their behavior
error BufferOverflow: i32 size, buffer buf {
    printf("Buffer overflow in %s, size: %d", buf, size)
    free(buf)
}

error FileNotFound: ->i8 path {
    printf("File not found: %s", path)
}

error PermissionDenied: ->i8 path {
    printf("Permission denied: %s", path)
}

// Use them
fn i32 read_config: ->i8 path {
    try {
        file = open(path)      // might throw FileNotFound or PermissionDenied
        data = read(file)      // might throw ReadError
        config = parse(data)   // might throw ParseError
        ret 0
    }
    catch {
        FileNotFound(path) ret -1,
        PermissionDenied(path) ret -2,
        BufferOverflow(1024, temp_buffer) ret -3,
        _ {
            printf("Unknown error in read_config")
            ret -99
        }
    }
}
```
### Q1: What causes an error?
```
if(errorCondition){
    throw ErrorName(args...)
}

```

### Q2: What's the default return value for void functions?

```tc
fn void log: {
    try { risky() }
    catch { BufferOverflow() ret void }  // no ret, returns void
}
```
In this case the default return value is `void`.

# new keywords for 1.3.2
+ `error` (type)
+ `throw` (statement)
+ `try` (statement)
+ `catch` (statement)

Total: 4 new keywords

- **18 keywords** — 
`if`, 
`loop`, 
`break`, 
`defer`, 
`ret`, 
`strun`, 
`fn`, 
`use`, 
`pub`, 
`pin`, 
`match`, 
`else`, 
`enum`, 
`async`, 
`select`,
`throw`,
`try`,
`catch`,
