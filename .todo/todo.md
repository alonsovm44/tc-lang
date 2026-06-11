
## later +1.4.0
[ ] Implement 1.4.0alpha features.
    - [ ] try/catch (mandatory)
    - [ ] comptime execution (mandatory)
    - [ ] FFI generator (can be defered if too difficult)
    - [ ] C/LLVM flags
        - [ ] Optimization flags
        - [ ] Debug 
        - [ ] Performance and Security 
        - [ ] Platform specific (medium importance)
        - [ ] Preprocessing and macros (can skip)
        - [ ] Linking (essential for making projects with C libs)

    - [ ] update stacks and queues
        - [ ] fixed size (buffered) stacks and queues
            - [ ] tested
        - [ ] multiple types
            - [ ] tested
        - [ ] wildcard
            - [ ] tested

[ ] Make a better, non trivial raylib demo and share it

# todo for 1.3.x
- [ ] HIGH PRIORITY: Fix async.h being included for non-async code
      - type_needs_runtime() incorrectly returns true for queue/stack
      - decl_needs_runtime() should only care about async functions
      - Test: queue/stack declaration without async should NOT include async.h
```bash
tigc source.tc -c app -o source.c # this brings async.h even if it wasnt called
```
- [s ] Fix bug where queues and stacks cant be freed when allocated
- [ ] Make Tig catch when we pass to few args or to many args to a function, not fallback on C to do it.
- [ ] add more methods to stacks and queues
    - [ ] size
    - [ ] clear
    - [ ] isEmpty <!-- maybe not needed, we could just check size -->

- [ ] Enforce 1.3 move semantics, moving is working without `@`
- [ ] Make multiple return values via queues/stacks work for sync functions.
- [ ] add inline braceless defer statements for ergonomics
```
defer free(ptr)
```
- [ ] Make extern C calls work for variables, not just for functions.
- [ ] Make inline C work in the global scope
- [ ] Make extern calls work with old and new syntax fn T,
- [ ] add multiline decls (enforce optional semicolon)
```
i32 a; i32 b;

i32 i; loop if(i < 100){} // more ergonomic
```
- [ ] Add a function to the stdlib that converts a fatptr string into a normal string
- [ ] Enforce everything being private by default, since pub does nothing since 1.1.
- [x] Fix defer not working
- [ ] Test ciclical imports on `use` stmt, make a #pragma once equivalent if needed

- [ ] Add multidimensional array support 
```
i32[8][8] chessboard = {}
i32[255][255][255] cube = {} 
```
>note: aparently this is supported already**
>note to self: test thoroughly**
- [ ] add boolean types, 0 or 1.
- [ ] add a dedicated module to the stdlib to manage queues and stacks
    - [ ] Make a method/lib to see if a queue/stack is blocked

- [done] FIX IO NOT WORKING, specifically reading files.
    - writef works.
    - openf works
    - fgetc is broken

- [ ] Implement pointers to stacks and queues
- [x] Implement stack and queue primitives
- [x] Implement basic stack and pointer features
    - [x] Stack operations (push, pop, peek)
    - [x] Queue operations (enq, deq, peek)
    - [ ] Pointer operations for stack and queue types

- [x] Implement async functions
    - [ ] Fix async functions
    > i forgot what is wrong with them. 
    > found it, the async runtime is not working, added notes on tests/async/

- [ ] explicit casting
```
i32 x = 5
f64 y = (f64)x
```

- [x] fix Tig compiler asking for ";" when it is optional
- [x] Fix emmiter to handle this case:
```
i32 x = 10
async foo(&x) // it produces bad code
```

- [x] fix the emitter to handle this case
```
i32 x = 110
->i32 ptr = &x
foo(ptr)

```
This compiles but hangs when ran

[x] debug why wc.tc hangs when ran (windows file locking issue) ( no, the problem is fgetc)

[ ] Exhaustively test 1.3.0 - 1.3.1 features before moving on to 1.4.0alpha
[ ] Expand stdlib to include web/http features
[ ] Build a small web server or non trivial program with Tig 1.3.1
---

Do we prefer implicit dereferencing for pointers (so q_ptr.push(val) just works) or explicit q_ptr.>push(val)?
- Explicit

Should the queue slice {0:2} be a read-only live view (Solution 2) or should it copy into a standard array slice =>i32 (Solution 1)?

- Read-only live view