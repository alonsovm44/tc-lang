# Issues that need to be addressed

1. Edge case: checker.c line 240 - missing null check before dereference
```c
if (e->left->type && (e->left->type->kind == TY_QUEUE...))
```
We check e->left->type but not e->left itself. If e->left is NULL, segfault.

2. The runtime is your weakest link right now. runtime.c has:

```c
ThreadPool *thread_pool_create(int thread_count) {
    (void)thread_count;
    return NULL;  // <-- returns NULL!
}
```
This means async programs will crash when thread_pool_submit dereferences NULL.

## No support for ' ' 
[ ] todo

This code breaks

f (seq(c, ' ') || seq(c, '\n') || seq(c, '\t'))

Tig does not recognize the ' ' quotes for comparing signle characters. 