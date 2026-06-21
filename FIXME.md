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


## FIX

## Extern "C" Block Bug (RESOLVED - Feature Removed)

**Status**: Extern blocks have been removed from Tig. The inline C approach is preferred.

**Solution**: Use inline C blocks instead:
```
"C"{
    #include <some_lib.h>
}

fn i32 my_printf: ->i8 fmt, ...{
    ret printf(fmt, ...)
}
```

This approach is simpler and removes a language feature. The entire file can include C headers via inline C blocks, and C functions can be called directly from Tig code without needing extern declarations.

**Changes made**:
- Removed `extern` keyword from keywords list in common.c
- Removed extern block parsing code from parser.c
- Removed DC_EXTERN_FN DeclKind from ast.h
- Removed is_extern field from Decl struct
- Updated parse_struct and parse_enum to remove is_extern parameter
- Updated all function calls to use new signatures
- Removed DC_EXTERN_FN handling in parse_fn
