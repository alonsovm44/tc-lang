# Spec 2.0.0+

## WebAssembly Backend

Tig compiles to webassembly

## Memory linter

Tig has a memory linter that tracks lifetimes and frees memory at compile time.

$ tigc --mem-lint main.tc

Warning: memory leak in function 'process' (main.tc:12)
  12 |     ->i32 ptr = alloc(i32, 100)
     |                ^^^^^^^^^^^^^^^^ allocation here
     |
     = note: ptr is not freed in this function and does not escape
     = help: add 'defer { free(ptr) }' before line 15

Warning: possible use-after-free in function 'use_ptr' (main.tc:25)
  25 |     free(ptr)
  26 |     printi(ptr[0])
     |            ^^^^^^ ptr used after free

Something like this would be really cool to have.