# TIG Compiler Directory

This directory contains the TIG language compiler source code - a source-to-source compiler (transpiler) written in ~4156 lines of C11.

## Directory Structure

```
compiler/
├── include/        Header files (interfaces for compiler components)
└── src/           Implementation files (compiler logic)
```

## Quick Navigation Guide

### For Understanding the Codebase

1. **Start here**: [DOCUMENTATION/COMPILER_ARCHITECTURE.md](../DOCUMENTATION/COMPILER_ARCHITECTURE.md)
   - Complete overview of compilation pipeline
   - Detailed description of each compiler phase
   - Data structures and their purposes

2. **Development guide**: [DOCUMENTATION/DEVELOPER_GUIDE.md](../DOCUMENTATION/DEVELOPER_GUIDE.md)
   - Step-by-step guide for adding new features
   - Code style guidelines
   - Debugging tips
   - Common pitfalls to avoid

### Compiler Phases

The compiler implements these phases in sequence:

1. **Lexer** (`src/lexer.c`, `include/lexer.h`)
   - Tokenizes source code
   - Handles macro expansion
   - Tracks line/column info

2. **Parser** (`src/parser.c`, `include/parser.h`)
   - Builds Abstract Syntax Tree (AST)
   - Recursive descent parsing
   - ~2000 lines

3. **Checker** (`src/checker.c`, `include/checker.h`)
   - Semantic analysis and type checking
   - Validates scopes and control flow
   - ~1500 lines

4. **Emitter** (`src/emitter.c`, `include/emitter.h`)
   - Generates C11 code from AST
   - ~1200 lines
   - Can also generate hot-swap code

5. **Runtime** (`src/runtime.c`, `include/runtime.h`)
   - Async support (thread pool, task scheduling)
   - Concurrent data structures
   - Only linked when async functions are used

### Header Files Overview

| File | Purpose | Key Types |
|------|---------|-----------|
| `ast.h` | AST definitions | `Type`, `Expr`, `Stmt`, `Decl` |
| `lexer.h` | Tokenization | `Token`, `TokenVec`, `TokenKind` |
| `parser.h` | Syntax analysis | `parse_program()` |
| `checker.h` | Semantic analysis | `check_program()` |
| `emitter.h` | Code generation | `emit_program()` |
| `common.h` | Utilities | Memory, strings, errors |
| `runtime.h` | Async support | Thread pool, queues, stacks |
| `io.h` | Error reporting | Pretty-printed diagnostics |

### Implementation Files Overview

| File | Purpose | Lines | Key Functions |
|------|---------|-------|----------------|
| `main.c` | Entry point | ~200 | CLI argument parsing, orchestration |
| `lexer.c` | Tokenization | ~600 | `lex_source()` |
| `parser.c` | Parsing | ~2000 | `parse_program()` and all parse_* functions |
| `checker.c` | Type checking | ~1500 | `check_program()` and all check_* functions |
| `emitter.c` | Code generation | ~1200 | `emit_program()` and all emit_* functions |
| `ast.c` | AST utilities | ~100 | Node creation and vector operations |
| `common.c` | Utilities | ~300 | Memory, strings, errors |
| `io.c` | Error reporting | ~100 | Pretty error formatting |
| `runtime.c` | Async runtime | ~400 | Thread pool, data structures |

## Building

### From project root

```bash
make                    # Build the compiler
make clean              # Remove build artifacts
```

This produces a `tigc` executable.

### Compiler invocations

```bash
./tigc input.tc -o output.c           # Transpile to C
./tigc input.tc -c output             # Transpile and compile
./tigc stdlib/io.tc -o stdlib/io.h    # Generate stdlib headers
```

## Key Concepts

### AST Structure

The compiler builds an AST with these node types:

- **Type**: Type information (i32, ->i32, =>i32, [10]i32, fn i32:..., queue<i32>)
- **Expr**: Expressions (literals, operators, calls, field access, etc.)
- **Stmt**: Statements (declarations, assignments, control flow, etc.)
- **Decl**: Top-level declarations (functions, structs, enums, imports, etc.)

All AST nodes are connected in a tree where the root is a `DeclVec` (vector of declarations).

### Symbol Tables

The checker maintains symbol tables for:
- **Functions**: Function name → signature and location
- **Variables**: Variable name → type and scope
- **Types**: Type name → type definition
- **Scopes**: Nested scopes for local variables

### Memory Management

All allocation uses safe wrappers:
- `xmalloc()`: Allocate or die
- `xrealloc()`: Reallocate or die
- `xstrdup()`: Copy string or die

No null checks needed - allocation failure exits immediately.

### Error Reporting

Errors include:
- Error code (E000, E001, etc.)
- Source location (line, column)
- Pretty-printed source context
- Colored output (when supported)

## Testing

```bash
# Run comprehensive tests
./tigc test.tc -c test
./test

# Compile samples
./tigc samples/fizzbuzz.tc -c fizzbuzz
./fizzbuzz

# Check specific features
./tigc samples/async.tc -o async.c  # Check async output
./tigc samples/defer.tc -o defer.c  # Check defer handling
```

## Common Tasks

### Add a new keyword/token type

1. Add token kind to `TokenKind` enum in `lexer.h`
2. Update lexer to recognize it in `lexer.c`
3. Update parser to handle it in `parser.c`
4. Add to keyword list in `common.c:is_keyword()`

### Add a new statement type

1. Add `ST_*` variant to `StmtKind` enum in `ast.h`
2. Add field to `struct Stmt` if needed
3. Add parser for it in `parser.c`
4. Add checker for it in `checker.c`
5. Add emitter for it in `emitter.c`

### Add a new expression type

Similar to statement types, update `ExprKind`, `struct Expr`, parser, checker, emitter.

### Debug compiler behavior

1. Add `printf()` statements in `main.c` for pipeline visibility
2. Use `-o` flag to save generated C for inspection
3. Compare with manually written C for expected behavior
4. Use `test.tc` or small samples for minimal test cases

## Performance Tips

- Compilation is single-pass (fast)
- Most expensive phase is usually code generation (string building)
- Symbol lookup is linear (could use hash tables for large programs)
- Token buffering is all-at-once (could stream for large files)

## Important Files to Know

- `main.c`: If you need to understand command-line processing
- `ast.h`: If you need to know all possible AST nodes
- `lexer.c`: If you need to add new syntax elements
- `parser.c`: If you need to understand grammar and AST construction
- `checker.c`: If you need to understand type checking and validation
- `emitter.c`: If you need to understand C code generation

## References

- **Language Specification**: See `DOCUMENTATION/TIG_LANGUAGE_SPECIFICATION.md`
- **Error Codes**: See `DOCUMENTATION/Errors.md`
- **Compiler Architecture**: See `DOCUMENTATION/COMPILER_ARCHITECTURE.md`
- **Developer Guide**: See `DOCUMENTATION/DEVELOPER_GUIDE.md`
- **Sample Programs**: See `samples/` directory
- **Tests**: See `test.tc` for comprehensive test suite

## Contributing

See [DOCUMENTATION/DEVELOPER_GUIDE.md](../DOCUMENTATION/DEVELOPER_GUIDE.md) for:
- Code style guidelines
- How to add new features
- Testing procedures
- Git workflow

## Questions?

1. Check existing code for similar implementations
2. Review the DEVELOPER_GUIDE.md
3. Look at test cases for usage examples
4. Check the COMPILER_ARCHITECTURE.md for high-level overview

