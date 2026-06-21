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

## Extern "C" Block Bug

**Problem**: The parser doesn't recognize the `extern` keyword at all when parsing extern "C" blocks. The lexer appears to be treating the entire block content as a single string instead of tokenizing it properly.

**Symptoms**:
- Test file `test_extern.tc` with `extern "C" { i32 x; }` fails with "expected 'fn', got 'x'"
- Test file with `extern "C" { struct Point { ... } }` fails with "'struct' is a keyword, not a valid identifier"
- The parser treats content inside extern blocks as if it's outside the block
- Error messages suggest the extern block parsing code (parser.c lines 1173-1207) is never triggered

**What was attempted**:
1. Added `is_extern` field to `Decl` struct in `ast.h`
2. Modified `parse_struct()` and `parse_enum()` to accept `is_extern` parameter
3. Updated all calls to these functions throughout parser.c
4. Modified extern block parsing in `parse_program_with_types()` to handle:
   - Struct declarations (`struct Name { ... }`)
   - Enum declarations (`enum Name { ... }`)
   - Variable declarations (`type name;`)
   - Function declarations (already supported)
5. Compiler builds successfully with only warnings
6. Test cases still fail with same errors

**Root cause hypothesis**:
The `extern` keyword itself is not being recognized by the parser. Either:
1. The lexer is not tokenizing `extern` as a keyword (check `is_keyword()` in common.c)
2. There's a parsing order issue preventing the `match(&p, "extern")` at parser.c:1173 from succeeding
3. Something earlier in the parsing flow is consuming the extern token

**Next steps for debugging**:
1. Verify `extern` is in the keywords list in `common.c` (it should be - line 206)
2. Add debug output to confirm if `match(&p, "extern")` is being called
3. Check if there's a check before line 1173 that's consuming the extern token
4. Verify the lexer is producing correct tokens for `extern "C" { ... }`
5. Test with a minimal case: just `extern "C" {}` to see if the block itself is recognized

**Workaround**: Use inline C blocks instead:
```
"C"{
    #include <some_lib.h>
}

fn void foo: ... {
    function(args...) // wrapping function from this lib
}
```
This works (see `stdlib/io.tc`) but requires wrapper functions for C API calls.

**Update**: The inline C approach may be sufficient without extern blocks. Example:
```
"C"{
    #include <stdio.h>
}

fn i32 my_printf: ->i8 fmt, ...{
    ret printf(fmt, ...)
}
```
This works because the inline C block includes headers, allowing direct C function calls from Tig code. Extern blocks may not be necessary at all.
