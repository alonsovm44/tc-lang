
# 1.4.0 alpha
[ ] Implement 1.4.0alpha features.
    - [x] try/catch (mandatory) [buggy]
    - [ ] comptime execution (mandatory)
    - [ ] FFI generator (can be defered if too difficult)
    - [ ] update stacks and queues
        - [ ] fixed size (buffered) stacks and queues
            - [ ] tested
        - [ ] multiple types
            - [ ] tested
        - [ ] wildcard
            - [ ] tested

[ ] Make a better, non trivial raylib demo and share it
# todo for 1.3.3

- [ ] Implement `map` type and library
- [ ]
# todo for 1.3.2
- [ ] C/LLVM flags
    - [ ] Optimization flags
    - [ ] Debug 
    - [ ] Performance and Security 
    - [ ] Platform specific (medium importance)
    - [ ] Preprocessing and macros (can skip)
    - [ ] Linking (essential for making projects with C libs)

# todo for 1.3.1

## Done
- [x] Separate stacks and queues from async to their own stdlib module
- [x] HIGH PRIORITY: Fix async.h being included for non-async code
- [x] Fix bug where queues and stacks cant be freed when allocated
- [x] Make Tig catch when we pass to few args or to many args to a function
- [x] add more methods to stacks and queues (size, clear, isEmpty)
- [x] Enforce 1.3 move semantics, moving is working without `@`
- [x] Make multiple return values via queues/stacks work for sync functions
- [x] add inline braceless defer statements for ergonomics
- [x] Make inline C work in the global scope
- [x] Make extern fn calls work with old and new syntax
- [x] Fix defer not working
- [x] add a dedicated module to the stdlib to manage queues and stacks
- [x] FIX IO NOT WORKING, specifically reading files
- [x] Implement stack and queue primitives
- [x] Implement basic stack and pointer features
- [x] Implement async functions
- [x] explicit casting
- [x] fix Tig compiler asking for ";" when it is optional
- [x] Fix emitter to handle async with pointer arguments
- [x] debug why wc.tc hangs when ran (windows file locking issue)

## Todo (sorted by importance for self-hosting)

### High Priority (essential for self-hosting)
- [x] Fix async functions (runtime not working)
    - Async works but it is fire and forget, we need a way to wait for results. [for now skip]
- [x] Fix if stmts not working in try blocks.
- [x] Implement pointers to stacks and queues
- [x] Pointer operations for stack and queue types
- [ ] Test ciclical imports on `use` stmt, make a #pragma once equivalent if needed
- [ ] Enforce everything being private by default, since `pub` does nothing since 1.1
- [ ] Make extern C calls work for variables, not just for functions

### Medium Priority (language features)
- [ ] add multiline decls (enforce optional semicolon)
- [x] Add a function to the stdlib that converts a fatptr string into a normal string
- [x] Add multidimensional array support (note: already supported, needs testing)
- [x] add boolean types, 0 or 1 (i8 is ok since it's just 0 or 1, C's bool is i8 anyway)

### Low Priority (nice to have)
- [ ] Exhaustively test 1.3.0 - 1.3.1 - 1.3.2 features before moving on to 1.4.0alpha
- [ ] Expand stdlib to include web/http features
- [ ] Build a small web server or non trivial program with Tig 1.3.1
---

# On design choices

Do we prefer implicit dereferencing for pointers (so q_ptr.push(val) just works) or explicit q_ptr.>push(val)?
- Explicit

Should the queue slice {0:2} be a read-only live view (Solution 2) or should it copy into a standard array slice =>i32 (Solution 1)?

- Read-only live view