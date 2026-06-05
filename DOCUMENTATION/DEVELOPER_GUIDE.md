# TIG Compiler Developer Guide

## Getting Started

### Prerequisites
- C11-compatible compiler (gcc, clang, or MSVC)
- Make (for building)
- Basic understanding of compiler design (lexing, parsing, code generation)

### Building the Compiler

```bash
# Navigate to the project directory
cd c:\Users\diego\.projects\langs\tig\tc

# Build the compiler
make

# Verify the build
./tigc --version  # (if implemented)
```

The output will be a `tigc` executable that can compile TIG programs.

---

## Project Structure

```
compiler/
├── include/           # Header files for compiler components
│   ├── ast.h         # Abstract Syntax Tree definitions
│   ├── checker.h     # Type checker interface
│   ├── common.h      # Common utilities and types
│   ├── emitter.h     # Code generator interface
│   ├── io.h          # I/O utilities
│   ├── lexer.h       # Tokenizer interface
│   ├── parser.h      # Parser interface
│   └── runtime.h     # Async runtime interface
└── src/              # Implementation files
    ├── ast.c         # AST node definitions and utilities
    ├── checker.c     # Semantic analysis and type checking
    ├── common.c      # Common utilities (memory, strings)
    ├── emitter.c     # C code generation
    ├── io.c          # Error reporting and I/O
    ├── lexer.c       # Tokenization
    ├── main.c        # Entry point and CLI handling
    ├── parser.c      # Parser implementation
    ├── runtime.c     # Async runtime implementation
    └── boot/         # Bootstrap code
```

---

## Code Organization

### Header Files (`include/`)

Each header file defines interfaces for a specific compiler component.

#### `ast.h` - Abstract Syntax Tree
Defines the data structures representing the program:
- `Type`: Type information
- `Expr`: Expressions
- `Stmt`: Statements
- `Decl`: Declarations (functions, structs, enums)
- Vector types: `ParamVec`, `ExprVec`, `StmtVec`, `DeclVec`

#### `lexer.h` - Tokenization
Defines:
- `Token`: Individual tokens with type, value, line, column
- `TokenVec`: Vector of tokens
- `TokenKind`: Token types (keywords, operators, literals, etc.)
- `lexer()`: Main tokenization function

#### `parser.h` - Parsing
Defines:
- `parse_program()`: Parse a complete program from tokens

#### `checker.h` - Type Checking
Defines:
- `check_program()`: Validate types and semantics

#### `emitter.h` - Code Generation
Defines:
- `emit_program()`: Generate C code from AST
- `emit_hot_split()`: Generate code for hot-swapping libraries

#### `common.h` - Utilities
Defines:
- Memory allocation: `xmalloc()`, `xrealloc()`
- String utilities: `xstrdup()`, `xstrndup()`, `str_push()`, `str_add()`, `str_printf()`
- Error reporting: `die()`, `tc_error()`, `tc_set_source()`
- Helper functions: `is_keyword()`, `is_type_name()`

#### `runtime.h` - Async Runtime
Defines the async runtime interface for thread pools and concurrent operations.

---

## Implementation Files (`src/`)

### `main.c` - Entry Point
Responsibilities:
- Parse command-line arguments
- Read input `.tc` file
- Call lexer, parser, checker, emitter in sequence
- Write output (`.c` file or invoke C compiler)
- Handle errors and report them to user

Entry point function: `main()`

### `lexer.c` - Tokenization
Key functions:
- `lexer()`: Main entry point, returns `TokenVec`
- `token_push()`: Add token to vector
- `two_char_symbol()`: Check for two-character operators
- Macro handling functions

Implementation strategy:
- Scan input character by character
- Recognize token patterns (keywords, literals, operators)
- Track position (line, column) for error reporting
- Handle escape sequences in strings

### `parser.c` - Syntax Analysis
Key functions:
- `parse_program()`: Parse declarations
- `parse_function()`: Parse function declaration
- `parse_struct()`: Parse struct definition
- `parse_statement()`: Parse a statement
- `parse_expression()`: Parse an expression
- Operator precedence handling

Implementation strategy:
- Recursive descent parsing
- Lookahead for decision making
- Build AST nodes bottom-up
- Maintain symbol table for forward references

### `checker.c` - Semantic Analysis
Key functions:
- `check_program()`: Check entire program
- `check_function()`: Validate function
- `check_statement()`: Validate statement
- `check_expression()`: Validate expression
- Type compatibility checking

Implementation strategy:
- Walk the AST recursively
- Maintain symbol tables (functions, variables, types)
- Type inference and checking
- Ownership validation for async code
- Detect use-after-free and double-free

### `emitter.c` - Code Generation
Key functions:
- `emit_program()`: Generate C code
- `emit_declaration()`: Generate declaration C code
- `emit_statement()`: Generate statement C code
- `emit_expression()`: Generate expression C code
- Runtime code generation

Implementation strategy:
- Walk validated AST
- Build C code string incrementally
- Track indentation for readability
- Generate function wrappers for async
- Include runtime headers when needed

### `ast.c` - AST Utilities
Responsibilities:
- Create and initialize AST nodes
- Copy AST nodes
- Free AST memory

### `common.c` - Common Utilities
Responsibilities:
- Memory allocation with error handling
- String manipulation
- Error reporting with pretty formatting
- Source context tracking

### `io.c` - I/O and Error Reporting
Responsibilities:
- Print error messages with colors
- Display source lines and carets
- Handle file I/O

### `runtime.c` - Async Runtime
Responsibilities:
- Thread pool implementation
- Task scheduling
- Queue and stack data structures
- Resource cleanup

---

## Adding a New Language Feature

### Example: Adding a New Statement Type

#### Step 1: Lexer
If the feature requires new tokens, add them to `lexer.h` and handle them in `lexer.c`:

```c
// In lexer.h
typedef enum {
    // ... existing tokens ...
    TOK_NEWKEYWORD,
} TokenKind;

// In lexer.c
if (strncmp(input, "newkeyword", 10) == 0) {
    // Recognize the new keyword
}
```

#### Step 2: AST
Add a new statement variant in `ast.h`:

```c
typedef enum {
    STMT_EXPR,
    STMT_RETURN,
    // ... existing statement types ...
    STMT_NEWFEATURE,
} StmtKind;

typedef struct {
    // New statement fields
    Expr *condition;
    Stmt *body;
} NewFeatureStmt;
```

#### Step 3: Parser
Add parsing logic in `parser.c`:

```c
static Stmt *parse_newfeature_stmt() {
    expect(TOK_NEWKEYWORD);
    Expr *condition = parse_expression();
    expect(TOK_LBRACE);
    Stmt *body = parse_statement();
    expect(TOK_RBRACE);
    
    NewFeatureStmt *nfs = xmalloc(sizeof(NewFeatureStmt));
    nfs->condition = condition;
    nfs->body = body;
    
    Stmt *stmt = xmalloc(sizeof(Stmt));
    stmt->kind = STMT_NEWFEATURE;
    stmt->data.newfeature = nfs;
    return stmt;
}
```

#### Step 4: Checker
Add validation in `checker.c`:

```c
static void check_newfeature_stmt(Stmt *stmt) {
    NewFeatureStmt *nfs = stmt->data.newfeature;
    
    // Validate condition type (should be boolean)
    Type *cond_type = check_expression(nfs->condition);
    if (!is_bool_type(cond_type)) {
        tc_error("E999", nfs->condition->line, nfs->condition->col, 
                 "Condition must be boolean");
    }
    
    // Validate body
    check_statement(nfs->body);
}
```

#### Step 5: Emitter
Add code generation in `emitter.c`:

```c
static void emit_newfeature_stmt(Str *out, Stmt *stmt) {
    NewFeatureStmt *nfs = stmt->data.newfeature;
    
    str_add(out, "if (");
    emit_expression(out, nfs->condition);
    str_add(out, ") {\n");
    emit_statement(out, nfs->body);
    str_add(out, "}\n");
}
```

#### Step 6: Test
Create test cases:

```tig
// In test.tc
i32 fn main { 
    i32 x = 5
    newfeature x > 3 {
        printi(1)
    }
    ret 0
}
```

---

## Debugging Tips

### 1. Enable Verbose Output
Modify `main.c` to print intermediate results:

```c
printf("DEBUG: Tokens generated: %d\n", tokens.count);
printf("DEBUG: Declarations parsed: %d\n", program.count);
```

### 2. Print AST
Write a function to print the AST structure:

```c
void print_ast_indent(Decl *decl, int indent) {
    printf("%*sDecl: %s (kind=%d)\n", indent, "", decl->name, decl->kind);
    // Print children recursively
}
```

### 3. Inspect Generated C
Always review the generated `.c` file:

```bash
./tigc input.tc -o output.c
cat output.c  # Inspect the generated code
```

### 4. Compare with Simple Examples
Test features with minimal programs first:

```bash
echo 'i32 fn main { ret 0 }' > minimal.tc
./tigc minimal.tc -c minimal
```

### 5. Use GDB
Debug the compiler itself:

```bash
gcc -g -o tigc_debug compiler/src/*.c
gdb ./tigc_debug
(gdb) run test.tc -o output.c
```

---

## Common Pitfalls

### 1. Not Freeing Memory
Always free AST nodes and other allocations:

```c
// Bad: Memory leak
Expr *expr = xmalloc(sizeof(Expr));
// ... use expr ...
// No free!

// Good: Proper cleanup
free_expr(expr);
```

### 2. Not Updating All Passes
If you add a new AST node kind, update:
- Parser (construct the node)
- Checker (validate it)
- Emitter (generate code for it)

Missing any pass will cause crashes or incorrect behavior.

### 3. Incorrect Line/Column Tracking
Always propagate line and column info:

```c
// Good: Preserve location info for errors
new_stmt->line = current_token->line;
new_stmt->col = current_token->col;

// Bad: Losing location info
new_stmt->line = 0;  // Error: can't report location
```

### 4. Type Mismatches
Be careful with type checking. Use helper functions:

```c
// Good: Use type comparison helpers
if (!types_compatible(actual, expected)) {
    // Error
}

// Bad: Naive comparison
if (actual->kind != expected->kind) {
    // Might miss equivalent types
}
```

### 5. Forgetting Special Cases
Remember async, pointers, and ownership:

```c
// Remember to handle:
// - Async functions (generate thread pool code)
// - Pointer types (->  and =>)
// - Ownership transfer (@)
// - Defer statements (generate cleanup)
// - Pin variables (immutability constraints)
```

---

## Testing

### Running Tests

```bash
# Compile test program
./tigc test.tc -c test

# Run test executable
./test
```

### Test Files

- `test.tc`: Comprehensive test suite
- `samples/`: Example programs demonstrating features
- `demos/`: More complex example applications

### Creating New Tests

Add test cases to `test.tc`:

```tig
// Test new feature
i32 fn test_newfeature {
    // Arrange
    i32 x = 5
    
    // Act
    // Test the feature here
    
    // Assert
    // Verify the result
    
    ret 0
}
```

---

## Performance Optimization

### Areas for Improvement

1. **String Building**: Consider using a string buffer pool
2. **Memory Allocation**: Profile allocations, consider slab allocation
3. **Symbol Lookup**: Use hash tables instead of linear search
4. **Token Buffering**: Stream processing instead of all-at-once
5. **Caching**: Cache type lookups and symbol resolution

---

## Documentation Standards

### Header Comments
```c
/**
 * function_name: Brief description
 * 
 * Detailed description explaining what the function does,
 * its parameters, return value, and any side effects.
 *
 * Args:
 *   param1: Description
 *   param2: Description
 * Returns:
 *   Description of return value
 */
```

### Inline Comments
```c
// Keep inline comments brief
// Explain the "why", not the "what"

// Bad: Obvious comment
i++  // Increment i

// Good: Explanatory comment
i++  // Skip invalid tokens to resync parser
```

### Type Comments
```c
typedef struct {
    char *name;       // Symbol name for lookup
    Type *type;       // Type information
    int line;         // Source line for error reporting
} Symbol;
```

---

## Git Workflow

### Commit Messages
```
component: Brief description (present tense)

Detailed explanation of the change, why it was made,
and any implications.

Related issue: #123
```

Example:
```
parser: Add support for async fn declarations

Previously, async functions were not parsed correctly.
This adds parsing support for the 'async' keyword before 'fn'
and creates appropriate AST nodes.

Checker and emitter updates will follow in separate commits.
```

---

## Resources

- **Compiler Design Books**: "Compilers: Principles, Techniques, and Tools" (Dragon Book)
- **C11 Standard**: http://www.open-std.org/jtc1/sc22/wg14/www/standards
- **TIG Specification**: See `DOCUMENTATION/TIG_LANGUAGE_SPECIFICATION.md`
- **Error Codes**: See `DOCUMENTATION/Errors.md`

---

## Getting Help

1. Check existing error codes in `Errors.md`
2. Review similar features in the codebase
3. Look at sample programs that use the feature
4. Check git history for related changes

---

## Code Style Guide

### Naming Conventions
- **Functions**: `snake_case` (e.g., `parse_expression()`)
- **Types**: `PascalCase` (e.g., `ExprVec`)
- **Constants**: `UPPER_CASE` (e.g., `DEFAULT_CAPACITY`)
- **Variables**: `snake_case` (e.g., `token_count`)

### Formatting
```c
// Braces on same line
if (condition) {
    statement;
}

// Function declarations
ReturnType function_name(ParamType param1, ParamType param2) {
    // implementation
}

// Struct definitions
typedef struct {
    Type field1;
    Type field2;
} StructName;
```

### Line Length
Keep lines under 100 characters for readability.

### Indentation
Use 4 spaces (not tabs).

