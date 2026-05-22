# Tight-C Error Reference

## Compile-time errors (source diagnostics)

### E000 — Assignment to pinned variable
Raised when code attempts to assign (`=`, `+=`, `-=`, etc.) to a variable that was previously pinned with `pin`.
```tc
i32 x = 10
pin x
x = 11   // E000: cannot assign to pinned variable 'x'
```

### E001 — Expected token not found
Raised when the parser expects a specific token (e.g. `{`, `}`, `)`, `:`) but encounters something else.
```tc
i32 fn add i32 a {   // E001: expected ':', got 'i32'
```

### E002 — Expected identifier
Raised when the parser expects a variable or function name but finds a non-identifier token.
```tc
i32 123 = 5   // E002: expected identifier, got '123'
```

### E003 — Keyword used as identifier
Raised when a reserved keyword is used where a variable or function name is expected.
```tc
i32 loop = 5   // E003: 'loop' is a keyword, not a valid identifier
```

### E004 — Expected expression
Raised when the parser expects an expression but finds an unexpected token.
```tc
i32 x = }   // E004: expected expression, got '}'
```

### E005 — Bare identifier statement
Raised when a lone identifier appears as a statement without any operation.
```tc
x   // E005: bare identifier 'x' is not a statement
```

### E006 — Bare literal statement
Raised when a lone literal value appears as a statement without any operation.
```tc
42   // E006: bare literal '42' is not a statement
```

### E007 — Invalid `@` directive
Raised when `@` appears at the top level but is not followed by `use`.
```tc
@import "lib.tc"   // E007: expected 'use' after '@'
```

### E008 — Invalid `@use` path
Raised when `@use` is not followed by a string literal path.
```tc
@use lib   // E008: '@use' expects a string path, got 'lib'
```

### E009 — File not found (`@use`)
Raised when the file specified by `@use` cannot be opened.
```tc
@use "missing.tc"   // E009: cannot open file 'missing.tc'
```

### E010 — Invalid `use` path
Raised when `use` is not followed by a string literal path.
```tc
use lib   // E010: 'use' expects a string path, got 'lib'
```

### E011 — Invalid `extern` block
Raised when `extern` is not followed by `"C"`.
```tc
extern "Rust" { }   // E011: extern expects "C", got '"Rust"'
```

### E012 — Unterminated literal
Raised when a block comment or string literal is opened but never closed.
```tc
/* this comment never ends
i32 x = "hello      // E012: unterminated string literal
```

### E013 — Unexpected character
Raised when the lexer encounters a character not part of the Tight-C grammar.
```tc
i32 x = 5 ¿        // E013: unexpected character '¿'
```

## CLI errors

### E100 — Missing input file
Raised when `tcc` is invoked without a source file argument.
```
$ tcc
usage: tcc <input.tc> [-o output.c] [-c binary]
```

### E101 — Missing argument after flag
Raised when `-o` or `-c` is provided without a following path.
```
$ tcc main.tc -o
missing output path after -o
```

### E102 — Cannot open file
Raised when the input source file cannot be read.
```
$ tcc nonexistent.tc
cannot open nonexistent.tc: No such file or directory
```

### E103 — No C compiler found
Raised when `-c` is used but no C compiler (`gcc`, `clang`, `cc`) is found on the system.
```
error: no C compiler found (tried gcc, clang, cc)
```

### E104 — C compilation failed
Raised when the underlying C compiler exits with a non-zero status.
```
error: C compilation failed (exit 1)
```