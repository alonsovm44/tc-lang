# Tig stdlib

This directory contains the standard library for the Tig programming language.

Format:
- Each module can have a corresponding .h file
- Each module can have a corresponding .tc file

How to write a stdlib module

1. Write extern C function calls to the C stdlib in the .tc file
2. Write structures and enums in the .tc file
3. Write functions/wrappers in the .tc file
    - If it is a libc wrapper, write the code to call the libc function
    - If it is not a wrapper, writ implementation in .tc
4. Transpile .tc source into .h file

Alternative

1. Write funcdefs and impl in the .h file and call the functions from .tc files
2. Migrate the impl code from the .h file to native Tig 
3. After migration, remove the .h file (optional)
4. Transpile .tc source into .h file