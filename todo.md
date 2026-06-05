Do we prefer implicit dereferencing for pointers (so q_ptr.push(val) just works) or explicit q_ptr.>push(val)?
- Explicit

Should the queue slice {0:2} be a read-only live view (Solution 2) or should it copy into a standard array slice =>i32 (Solution 1)?

- Read-only live view

# todo

- [ ] Implement stack and queue primitives
- [ ] Implement pointers to stacks and queues
- [ ] Implement basic stack and pointer features
    - [ ] Stack operations (push, pop, peek)
    - [ ] Queue operations (enq, deq, peek)
    - [ ] Pointer operations for stack and queue types

- [x] Implement async functions [DONE]

- [ ] explicit casting
```
i32 x = 5
f64 y = (f64)x
```

- [ ] fix Tig compiler asking for ";" when it is optional
- [ ] add multiple line code
```tc
a; b; c;
```