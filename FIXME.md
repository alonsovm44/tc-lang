# Issues that need to be addressed

Edge case: checker.c line 240 - missing null check before dereference
```c
if (e->left->type && (e->left->type->kind == TY_QUEUE...))
```
We check e->left->type but not e->left itself. If e->left is NULL, segfault.
