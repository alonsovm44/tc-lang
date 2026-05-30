# Hot function reloading
Here's the core idea:

tC's hot reload lets you mark any function with hot, and the compiler automatically splits your program into two parts: a stable executable containing main() and function stubs, plus a shared library (.so/.dll) containing the actual implementations of all hot functions. When you edit a hot function and recompile, only the tiny shared library rebuilds , the executable keeps running, detects the file change, and seamlessly swaps in the new version using dlopen/LoadLibrary. Your program's state (variables, heap data, open files) never resets.

This is a breakthrough for systems programming because it kills the brutal edit-compile-run cycle. Game developers, embedded engineers, and trading system programmers currently wait minutes to hours to see the effect of a one-line change. With tC, the same change takes milliseconds. No other C-class language offers trivial hot reload, you'd be the first to make it as simple as writing hot fn instead of fn. That's not just convenience; it's a 100x-1000x improvement in iteration speed for the most demanding programming domains.

Example 

```
use "stdlib/io.tc"

fn i32 add: i32 x, i32 y{
    return x + y;
}
    
fn void main: {
    i32 x = 3
    i32 y = 4
    i32 result = add(x, y)
    loop:
        printi(result)
}
```
The normal way would be
`tcc source.tc -c app` or `tcc source.tc -o source.c -c app` 
This creates the transpiled source plus executable. 
Any changes done we have to recompile and re run the executable, and do the required inputs to put the program in the state we want to test.
Program output
```
./app
7
7
7
7
7
7
7
...
```

## With hot functions

```
use "stdlib/io.tc"

hot fn i32 add: i32 x, i32 y{
    return x + y;
}
    
fn void main: {
    i32 x = 3
    i32 y = 4
    i32 result = add(x, y)
    printi(result)
}
```

We run `tcc source.tc -h hotfns.so -c app` and it creates 
The compiler automatically splits your program into two parts: a stable executable containing main() and function stubs, plus a shared library (.so/.dll) containing the actual implementations of all hot functions.

We get

source.tc 

hotfns.so/dll
source_c.c
app.exe

The we can run `app.exe` and it will load the hot functions from `hotfns.so/dll`.

We run `app.exe`
Output:
```
7
7
7
7
7
...
```

Now we modify a the hot fn while app.exe / app prints 7 for ever

```
hot fn add: i32 x, i32 y {
    ret x + y + 10 // we add 10
}

... // rest of the program remains the same
```

`tcc source.tc -h hotfns.so -c app`

The compiler automatically splits your program into two parts:


source.tc
    ↓
    ├── app.exe (stable executable with main() + stubs)
    └── hotfns.so (shared library with hot function implementations)
Run the executable:

bash
./app
Output:

text
7
7
7
7
7
...
Hot Reload in Action
While the program is still running (printing 7s forever), you edit the hot function:

```
hot fn add: i32 x, i32 y {
    ret x + y + 10    // Added 10
}

// The rest of the program remains the same
```

- Now rebuild only the shared library:

`tcc source.tc -h hotfns.so --hot-rebuild`

The running program automatically detects the file change and loads the new version. Output changes immediately:

text
7
7
7
17      <- Changed without restart!
17
17
17
...
The executable never stopped. The program state (x, y, loop position) never reset.

## Conclusion 
This should be enough for a POC for version 1.2

## Future Plans for 1.3+
1. Automatic struct migration when fields change
2. Networked hot reload (update remote systems)
3. Hot reload visualization tool (show which functions changed)
4. Rollback support (revert to previous version if new code crashes)

## keywords for 1.2.0

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
12. hot (new)
