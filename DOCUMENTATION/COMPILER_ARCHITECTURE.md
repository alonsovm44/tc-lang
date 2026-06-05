# TIG Compiler Architecture

## Overview

The TIG compiler is a **source-to-source compiler (transpiler)** that translates TIG language `.tc` files into portable C11 code. It's implemented in ~4156 lines of C11 and follows a classic multi-stage compiler design.

## Compilation Pipeline

```
Input (.tc file)
    ↓
[Lexer] → Tokenization
    ↓
[Parser] → Abstract Syntax Tree (AST) Construction
    ↓
[Checker] → Semantic Analysis & Type Checking
    ↓
[Emitter] → C11 Code Generation
    ↓
[C Compiler] (gcc/clang) → Binary Output
```

## Architecture Components

### 1. **Lexer** (`lexer.h`, `lexer.c`)
**Purpose**: Converts raw source code text into a stream of tokens.

**Key Responsibilities**:
- Tokenize the input source code
- Recognize keywords, identifiers, literals, and operators
- Track line and column information for error reporting
- Handle macro expansion (simple text substitution)
- Manage two-character operators (`->`, `=>`, `==`, etc.)

**Key Data Structures**:
- `Token`: Represents a single token with type, value, line, and column info
- `TokenVec`: Dynamic array of tokens
- `Macro`: Represents macro definitions (both simple and parametric)

**Token Types**:
- Keywords: `if`, `loop`, `break`, `defer`, `ret`, `fn`, `use`, `pub`, `pin`, `match`, `else`, `enum`, `async`, `select`, `strun`
- Literals: numbers, strings, character constants
- Identifiers: variable and function names
- Operators: arithmetic, comparison, assignment, logical, bitwise
- Delimiters: `{`, `}`, `(`, `)`, `;`, `,`, `:`, etc.

---

### 2. **Parser** (`parser.h`, `parser.c`)
**Purpose**: Constructs an Abstract Syntax Tree (AST) from the token stream.

**Key Responsibilities**:
- Parse program structure (declarations and statements)
- Handle function definitions, type declarations, struct definitions
- Parse expressions with proper operator precedence
- Manage control flow (if/else, loops, match, select)
- Support async functions and async operations
- Parse type annotations (raw pointers, fat pointers, arrays, function pointers, generics)

**Key Data Structures**:
- `Decl`: Represents declarations (functions, structs, variables)
- `Stmt`: Represents statements (assignments, calls, control flow)
- `Expr`: Represents expressions (literals, identifiers, operators, function calls)
- `Type`: Represents type information with kind and additional data

**Grammar Highlights**:
```
Program → (Declaration)*
Declaration → FunctionDecl | StructDecl | VariableDecl
FunctionDecl → RetType 'fn' Name ':' Parameters '{' Statements '}'
Type → TypeName | RawPointer | FatPointer | Array | FunctionPointer | Generic
```

---

### 3. **Checker** (`checker.h`, `checker.c`)
**Purpose**: Performs semantic analysis and type checking on the AST.

**Key Responsibilities**:
- Validate type correctness
- Check variable and function declarations
- Verify function signatures and call compatibility
- Validate struct field access and indexing operations
- Check control flow and return statements
- Validate async operations and ownership transfer (`@` operator)
- Detect undefined variables and functions
- Handle forward declarations for recursive functions

**Key Validations**:
- Type compatibility for assignments and operations
- Function call argument type checking
- Struct field existence and type matching
- Pointer operations and array indexing
- Control flow completeness (all code paths return values)

---

### 4. **Emitter** (`emitter.h`, `emitter.c`)
**Purpose**: Generates C11 code from the checked AST.

**Key Responsibilities**:
- Transform AST nodes into C11 code
- Generate struct and type definitions
- Emit function implementations
- Handle pointer conversions and type casts
- Generate async runtime code when needed
- Support hot-swapping library splitting (for runtime reloading)
- Include necessary headers and runtime support
- Generate appropriate memory management code

**Key Features**:
- **Code Generation**: Directly emits C code from AST
- **Runtime Integration**: Automatically includes async runtime when async functions are detected
- **Memory Management**: Generates `alloc()` and `free()` calls as appropriate
- **Ownership Transfer**: Handles `@` operator for transferring resources
- **Type Casting**: Generates safe conversions between compatible types

**Special Handling**:
- Inline C blocks: Pass through to output C code
- Macros: Inline macro replacements in the output
- Defer statements: Transform into cleanup code before returns

---

### 5. **Runtime** (`runtime.h`, `runtime.c`)
**Purpose**: Provides runtime support for async operations and standard library features.

**Key Features**:
- Thread pool management for async execution
- Queue and stack data structures for concurrent programming
- Automatic initialization on first async call
- Clean shutdown and resource cleanup
- Built-in memory allocation tracking

**When Used**:
- Automatically linked only when async functions are detected
- Zero overhead for pure synchronous programs

---

## Key Data Structures

### AST Nodes
```c
Type          // Type information (kind, base type, array size, etc.)
Param         // Function parameter (type + name)
Expr          // Expression (literals, operations, function calls, etc.)
Stmt          // Statement (assignments, declarations, control flow)
Decl          // Top-level declaration (function, struct, enum, variable)
```

### Vector Types (Dynamic Arrays)
```c
ParamVec      // Vector of parameters
ExprVec       // Vector of expressions
StmtVec       // Vector of statements
DeclVec       // Vector of declarations
TokenVec      // Vector of tokens
MacroVec      // Vector of macros
```

---

## Compilation Stages in Detail

### Stage 1: Lexical Analysis (Lexer)
Input: Raw `.tc` source file
Output: `TokenVec` (sequence of tokens)

Example:
```tig
i32 fn add: i32 a, i32 b { ret a + b }
```
Becomes tokens: `[i32, fn, add, :, i32, a, ,, i32, b, {, ret, a, +, b, }]`

---

### Stage 2: Syntax Analysis (Parser)
Input: `TokenVec`
Output: `DeclVec` (AST of declarations)

The parser builds a tree structure representing the program:
```
Program
  └─ FunctionDecl("add")
      ├─ RetType: i32
      ├─ Params: [i32 a, i32 b]
      └─ Body: [ReturnStmt(BinaryOp(a, +, b))]
```

---

### Stage 3: Semantic Analysis (Checker)
Input: `DeclVec` (AST)
Output: Validated `DeclVec` or error messages

The checker:
- Validates that all referenced variables are declared
- Ensures type compatibility in operations
- Checks function calls have correct argument types
- Verifies control flow (all paths return appropriate types)
- Detects use-after-free and ownership violations

---

### Stage 4: Code Generation (Emitter)
Input: Validated `DeclVec` (AST)
Output: C11 source code (string)

The emitter converts each AST node to equivalent C code:
```tig
i32 fn add: i32 a, i32 b { ret a + b }
```
Becomes:
```c
i32 add(i32 a, i32 b) {
  return a + b;
}
```

---

### Stage 5: C Compilation
Input: Generated C11 code
Output: Binary executable

Uses the system C compiler (gcc, clang, MSVC) to compile the output.

---

## Error Handling

**Error Reporting Pipeline**:
1. Errors are reported with specific error codes (E000, E001, etc.)
2. Source context is maintained from the input file
3. Line and column information is preserved through compilation
4. Error messages include colored diagnostics with source lines and carets
5. Compilation stops at the first semantic error

**Error Categories**:
- **Parse Errors**: Invalid syntax
- **Type Errors**: Type mismatches in operations
- **Semantic Errors**: Undefined variables, invalid operations
- **Async Errors**: Invalid async usage, ownership issues

---

## Special Features

### Async Runtime
When the emitter detects async functions (`async fn`), it:
1. Includes the async runtime headers
2. Generates thread pool initialization
3. Emits async task handling code
4. Links against the async runtime

### Hot-Swapping (Dynamic Reloading)
The emitter supports splitting a program for hot-swapping:
- `emit_hot_split()` generates separate `.c` and `.h` files
- Allows runtime library reloading without restarting
- Used for demo applications requiring dynamic updates

### Macro System
- **Simple Macros**: Text substitution during lexing
- **Parametric Macros**: Macros with parameters, expanded at usage sites
- Macros are expanded before parsing for transparent integration

---

## Build System

The compiler is built with `make`:

```bash
make          # Build the compiler (produces tigc executable)
make clean    # Remove build artifacts
```

The compiled `tigc` executable can then:
- Transpile `.tc` to `.c`: `./tigc input.tc -o output.c`
- Transpile and compile: `./tigc input.tc -c output`
- Compile stdlib: `./tigc stdlib/io.tc -o stdlib/io.h`

---

## Development Workflow

### Adding a New Feature
1. **Lexer**: Add token recognition if needed
2. **Parser**: Add grammar rules for new syntax
3. **AST**: Add new `Expr`, `Stmt`, or `Decl` variants if needed
4. **Checker**: Add validation logic
5. **Emitter**: Add C code generation
6. **Tests**: Add test cases in `test.tc` or sample files

### Debugging
- Use `-v` flag for verbose output (if implemented)
- Check error messages for the exact error location
- Review generated `.c` files for correctness
- Test with simple examples in `samples/`

---

## Performance Characteristics

- **Compilation Speed**: Fast single-pass compilation (no multiple passes)
- **Binary Size**: Minimal overhead, only runtime included when needed
- **Runtime Overhead**: Zero for synchronous programs
- **Memory Usage**: All async operations use thread pool (no unbounded thread creation)

---

## Language Semantics in Code

### Type System
- **Type Kinds**: Named types, raw pointers (`->`), fat pointers (`=>`), arrays, function pointers, generics (`queue<T>`, `stack<T>`)
- **Memory**: Manual with `alloc()`/`free()`, `defer` for cleanup
- **Ownership**: Explicit with `@` operator for async ownership transfer

### Control Flow
- **If/Else**: Standard conditional branching
- **Loop**: Infinite loops with `break` for exit
- **Match**: Pattern matching on enum/union values
- **Select**: Wait on multiple async operations
- **Defer**: Register cleanup to run before function exit

### Async
- **Zero Setup**: Automatic runtime initialization
- **Simple Syntax**: `async fn` keyword for async functions
- **Ownership**: `@` operator for transferring ownership to tasks
- **Pin**: `pin` keyword to keep variables alive across async boundaries
- **Select**: Wait for multiple async operations

---

## Limitations and Known Issues

1. **Queue/Stack Bugs**: Concurrent data structures have known issues
2. **Single-Pass**: No separate type inference pass (all types must be explicit)
3. **No Generics**: Generic types are limited to built-in collections
4. **Error Recovery**: Compiler stops at first error (no error recovery)
5. **C Backend**: Limited to C11 capabilities (no features beyond C11)

---

## References

- Language Specification: See `TIG_LANGUAGE_SPECIFICATION.md`
- Sample Programs: See `samples/` directory
- Error Codes: See `DOCUMENTATION/Errors.md`
- Test Cases: See `test.tc` and test files

