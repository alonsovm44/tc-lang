# 1.3.2 spec
CLI update

## Multiple file compilation

The compiler can now compile multiple files at once. The rules are:
  - Only one file can have main function, linker error otherwise
  - Other files are treated as libs
  - Compiler merges ASTs from all files
  - main from whichever file has it becomes the entry point.

```bash 
tigc file1.tc file2.tc -c app 

```

## C compiler flags

The compiler can now pass flags to the C compiler using --

```bash
tigc file1.tc file2.tc -c app -- -O2 -lraylib -L/usr/lib
```
