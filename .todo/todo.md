Do we prefer implicit dereferencing for pointers (so q_ptr.push(val) just works) or explicit q_ptr.>push(val)?
- Explicit

Should the queue slice {0:2} be a read-only live view (Solution 2) or should it copy into a standard array slice =>i32 (Solution 1)?

- Read-only live view

# todo
- [ ] Enforce everything being private by default, since pub does nothing since 1.1.
- [ ] Fix defer not working
- [ ] Test ciclical imports on `use` stmt, make a #pragma once equivalent if needed
- [ ] Add multidimensional array support 
**note: aparently this is supported already**
**note to self: test thoroughly**
```
i32[8][8] chessboard = {}
i32[255][255][255] cube = {} 
```
- [ ] add boolean types, 0 or 1.
- [ ] add a dedicated module to the stdlib to manage queues and stacks
    - [ ] Make a method/lib to see if a queue/stack is blocked
- [ ] FIX IO NOT WORKING, specifically reading files.

- [x] Implement stack and queue primitives
- [ ] Implement pointers to stacks and queues
- [x] Implement basic stack and pointer features
    - [x] Stack operations (push, pop, peek)
    - [x] Queue operations (enq, deq, peek)
    - [ ] Pointer operations for stack and queue types

- [x] Implement async functions
    - [ ] Fix async functions

- [ ] explicit casting
```
i32 x = 5
f64 y = (f64)x
```

- [x] fix Tig compiler asking for ";" when it is optional
- [ ] add multiple line code
```tc
a; b; c;
```
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

[x] debug why wc.tc hangs when ran (windows file locking issue)
[ ] Implement 1.3.1 and 1.4.0alpha features.
    - [ ] try/catch
    - [ ] comptime execution
    - [ ] FFI generator
    - [ ] C flags
    - [ ] update stacks and queues

[ ] Exhaustively test 1.3.0 - 1.3.1 features
[ ] Expand stdlib to include web/http features
    [ ] Make stdlib IO bullet proof
[ ] Build a small web server or non trivial program with Tig
[ ] Make a better, non trivial raylib demo and share it
