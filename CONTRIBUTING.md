# Contributing to Tight-C

Thank you for your interest in contributing to Tight-C! This guide will help you get started with contributing to the compiler.

## Overview

Tight-C is a systems programming language that transpiles to C. It focuses on simplicity, performance, and developer productivity with features like hot reloading, methods in structs, and modern language constructs.

## Rules
1. AI codegen is OK as long as it is C11 for the C core
2. Follow the rules displayed in the stdlib section if you want to contribute to the stdlib
3. NO AI CODEGEN for self hosting code. PRs with AI generated code for stage1 will not be taken into account.
4. Have fun!

## Development Setup

### Prerequisites

- GCC or Clang compiler
- Make build system
- Git
- Basic understanding of C programming and compiler concepts

### Building the Compiler

```bash
git clone <repository-url>
cd tc
make          # Build tightc
make clean    # Remove build artifacts
```

### Testing Your Changes

```bash
# Test compilation
./tightc samples/hello_world.tc

# Test with C compilation
./tightc samples/hello_world.tc -o test.c
gcc -std=c11 test.c -o test
./test

# Test method functionality
./tightc samples/methods_test.tc -c methods
./methods
```

## Project Structure

```
tc/
├── compiler/
│   ├── include/          # Header files
│   │   ├── ast.h         # AST definitions
│   │   ├── checker.h     # Type checker
│   │   ├── common.h      # Common utilities
│   │   ├── emitter.h     # C code generation
│   │   ├── io.h          # I/O utilities
│   │   ├── lexer.h       # Lexical analysis
│   │   └── parser.h      # Parsing
│   └── src/              # Source files
│       ├── ast.c         # AST implementation
│       ├── checker.c     # Type checking
│       ├── common.c      # Common utilities
│       ├── emitter.c     # C code generation
│       ├── io.c          # I/O utilities
│       ├── lexer.c       # Lexical analysis
│       ├── main.c        # CLI interface
│       └── parser.c      # Parsing
├── samples/              # Test files and examples
├── docs/                  # Documentation
└── stdlib/               # Standard library
```

## Compiler Architecture

The Tight-C compiler follows a traditional pipeline architecture:

1. **Lexer** (`lexer.c`) - Converts source code into tokens
2. **Parser** (`parser.c`) - Builds AST from tokens
3. **Checker** (`checker.c`) - Type checking and semantic analysis
4. **Emitter** (`emitter.c`) - Generates C code from AST

### Key Data Structures

- **AST Nodes** (`ast.h`) - Abstract syntax tree representations
- **Tokens** (`lexer.h`) - Lexical tokens with position information
- **Symbols** - Variable and function tracking in scopes

## Adding New Features

### 1. Language Syntax

When adding new language features:

1. **Update the lexer** if new tokens are needed
2. **Update the parser** to recognize new syntax
3. **Update the AST** if new node types are needed
4. **Update the checker** for type validation
5. **Update the emitter** for C code generation
6. **Add comprehensive tests**

### 2. Example: Adding a New Expression Type

```c
// ast.h - Add new expression kind
typedef enum {
    // ... existing kinds
    EX_NEW_EXPRESSION,
} ExprKind;

// ast.c - Add constructor
Expr *new_new_expression(Type *type) {
    Expr *e = new_expr(EX_NEW_EXPRESSION);
    e->type = type;
    return e;
}

// parser.c - Add parsing logic
static Expr *parse_new_expression(Parser *p) {
    expect(p, "new");
    Type *type = parse_type(p);
    return new_new_expression(type);
}

// checker.c - Add type checking
case EX_NEW_EXPRESSION:
    check_expr(e->type, s);
    break;

// emitter.c - Add code generation
case EX_NEW_EXPRESSION:
    str_add(out, "malloc(sizeof(");
    emit_type(out, e->type, "", program);
    str_add(out, "))");
    break;
```

## Coding Standards

### C Code Style

- Use 4-space indentation (no tabs)
- Opening braces on same line for functions, new line for control structures
-camelCase for variable names, PascalCase for types
- Prefix static functions with descriptive names
- Include function documentation for complex logic

```c
// Good example
static Expr *parse_binary_expression(Parser *p, int precedence) {
    // Parse left operand
    Expr *left = parse_primary_expression(p);
    
    // Parse operators while precedence allows
    while (current_precedence(p) >= precedence) {
        Token *op = expect_operator(p);
        Expr *right = parse_binary_expression(p, precedence + 1);
        left = new_binary_expression(op, left, right);
    }
    
    return left;
}
```

### Error Handling

- Use `tc_error()` for compilation errors with error codes
- Provide clear, helpful error messages with context
- Include line and column information in error reports
- Use consistent error code format (E001, E002, etc.)

```c
// Good error handling
if (!type) {
    tc_error("E014", token->line, token->col, 
             (int)strlen(token->text),
             "undefined type '%s'", token->text);
}
```

## Testing

### Test Categories

1. **Unit Tests** - Individual compiler components
2. **Integration Tests** - Complete compilation pipeline
3. **Samples** - Real-world usage examples
4. **Regression Tests** - Known fixed issues

### Writing Tests

- Add test cases to `samples/` directory
- Use descriptive test names
- Test both success and failure cases
- Include edge cases and error conditions

```tc
// samples/test_new_feature.tc
use "stdlib/io.tc"

strun TestStruct {
    i32 value
    
    fn i32 getValue: {
        ret self.>value
    }
}

fn void main: {
    TestStruct test
    test.value = 42
    printi(test.getValue())  // Should print 42
}
```

## Documentation

### Updating Documentation

1. **Feature Documentation** - Update `docs/1.2.0-full.md`
2. **API Documentation** - Update relevant sections
3. **Examples** - Add working samples to `samples/`
4. **Changelog** - Update `CHANGELOG.md`

### Documentation Style

- Use clear, concise language
- Include code examples for all features
- Show generated C code where relevant
- Follow existing formatting patterns

## Submitting Changes

### Pull Request Process

1. Fork the repository
2. Create a feature branch (`git checkout -b feature-name`)
3. Make your changes with tests
4. Ensure all existing tests pass
5. Update documentation
6. Submit a pull request

### Pull Request Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing
- [ ] All existing tests pass
- [ ] New tests added for new functionality
- [ ] Manual testing completed

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
```

## Common Issues

### Build Failures

- Ensure GCC/Clang is installed and in PATH
- Check for missing dependencies
- Clean build with `make clean && make`

### Test Failures

- Check generated C code for syntax errors
- Verify AST structure in debugger
- Use `--error` flag to understand error codes

### Performance Issues

- Profile with standard C profiling tools
- Check for memory leaks in AST construction
- Optimize hot paths in lexer/parser

## Debugging Tips

### Compiler Debugging

```bash
# Enable verbose output
./tightc --debug source.tc

# Check generated C code
./tightc source.tc -o output.c

# Test specific error codes
./tightc --error E014
```

### Common Debugging Areas

- Token positions in lexer
- AST node construction in parser
- Type propagation in checker
- C code generation in emitter

## Community

### Getting Help

- Check existing documentation first
- Review test cases for similar patterns
- Examine generated C code for debugging
- Use error codes to understand issues

### Contributing Guidelines

- Start with small, well-defined changes
- Focus on one feature at a time
- Write tests before implementation
- Document your changes clearly

## Release Process

The project follows semantic versioning:

- **Major** (X.0.0) - Breaking changes
- **Minor** (X.Y.0) - New features
- **Patch** (X.Y.Z) - Bug fixes

Version updates require:
- Updated version in `main.c`
- Updated CHANGELOG.md
- Updated documentation
- Full test suite pass

Thank you for contributing to Tight-C! Your contributions help make systems programming more accessible and productive.
