# TIG Abstract Syntax Tree (AST) Guide

## Overview

The Abstract Syntax Tree (AST) is the core data structure that represents a TIG program after parsing. It captures the program's structure and relationships in a tree format that's easy to analyze and transform.

The AST is produced by the **parser** and consumed by the **checker** and **emitter**.

## Key Components

### 1. Declarations (Decl)

Top-level declarations represent the program's definitions:

```c
struct Decl {
    DeclKind kind;           // What kind of declaration
    bool public;             // Exported with 'pub'?
    bool is_hot;             // Hot-reload function?
    bool is_async;           // Async function? (v1.3)
    char *name;              // Declaration name
    char *path;              // Import path (for DC_USE)
    Type *type;              // Return type (for functions) or type (for vars)
    Expr *init;              // Initial value (for variables)
    ParamVec params;         // Parameters (for functions/structs)
    StmtVec body;            // Function body
    bool varargs;            // Accepts variable args?
    bool packed;             // Struct is packed (no padding)?
    EnumMemberVec enum_members;  // Enum variants
    DeclVec methods;         // Methods (for structs)
    char *text;              // Raw C code (for inline C)
}
```

#### Declaration Kinds

| Kind | Purpose | Example |
|------|---------|---------|
| `DC_USE` | Import another file | `use "stdlib/io.tc"` |
| `DC_STRUCT` | Define struct type | `strun Point { i32 x, i32 y }` |
| `DC_ENUM` | Define enum type | `enum Color { Red, Green, Blue }` |
| `DC_FN` | Define function | `i32 fn add: i32 a, i32 b { ret a + b }` |
| `DC_VAR` | Global variable | `i32 x = 42` |
| `DC_EXTERN_FN` | C function declaration | `extern "C" void free: ->void ptr` |
| `DC_INLINE_C` | Raw C code | `%{ #include <stdio.h> %}` |

#### Declaration Examples

```tig
// Function declaration becomes:
i32 fn add: i32 a, i32 b { ret a + b }

Decl {
    kind: DC_FN,
    name: "add",
    type: (i32),
    params: [Param(i32, "a"), Param(i32, "b")],
    body: [Stmt(ST_RET, expr: BinaryOp(a, +, b))]
}
```

```tig
// Struct declaration becomes:
strun Point {
    i32 x
    i32 y
}

Decl {
    kind: DC_STRUCT,
    name: "Point",
    packed: false,
    params: [Param(i32, "x"), Param(i32, "y")]
}
```

### 2. Statements (Stmt)

Statements are instructions within function bodies:

```c
struct Stmt {
    StmtKind kind;           // What kind of statement
    Type *type;              // Type annotation (for vars)
    char *name;              // Variable name (for vars)
    Expr *expr;              // Primary expression
    Expr *expr2;             // Secondary expression
    StmtVec body;            // Nested statements
    MatchArmVec arms;        // Match cases
    ElseIfVec elseifs;       // If/else-if branches
    StmtVec else_body;       // Final else block
    char *text;              // Inline C code
}
```

#### Statement Kinds

| Kind | Purpose | Example |
|------|---------|---------|
| `ST_VAR` | Variable declaration | `i32 x = 5` |
| `ST_BLOCK` | Code block | `{ stmt1; stmt2; }` |
| `ST_IF` | Conditional | `if condition { } else { }` |
| `ST_LOOP` | Loop | `loop { break }` |
| `ST_DEFER` | Cleanup | `defer free(ptr)` |
| `ST_RET` | Return | `ret value` |
| `ST_BREAK` | Break loop | `break` |
| `ST_PIN` | Pin variable | `pin x` |
| `ST_EXPR` | Expression statement | `func_call()` |
| `ST_MATCH` | Pattern match | `match x { 1 { } 2 { } }` |
| `ST_SELECT` | Async wait | `select { op1 { } op2 { } }` |
| `ST_INLINE_C` | Raw C code | `%{ printf(...); %}` |

#### Statement Examples

```tig
// Variable declaration becomes:
i32 x = 5

Stmt {
    kind: ST_VAR,
    type: (i32),
    name: "x",
    expr: Expr(EX_LITERAL, text: "5")
}
```

```tig
// If statement becomes:
if x > 0 {
    ret 1
} else {
    ret 0
}

Stmt {
    kind: ST_IF,
    expr: Expr(EX_BINARY, left: x, op: >, right: 0),
    body: [Stmt(ST_RET, expr: 1)],
    else_body: [Stmt(ST_RET, expr: 0)]
}
```

### 3. Expressions (Expr)

Expressions compute values:

```c
struct Expr {
    ExprKind kind;           // What kind of expression
    char *text;              // Text content
    Type *type;              // Type (filled by checker)
    Expr *left;              // Left operand
    Expr *right;             // Right operand
    Expr *third;             // Third operand (rarely used)
    ExprVec args;            // Function arguments
    int line, col;           // Source location
}
```

#### Expression Kinds

| Kind | Purpose | Structure | Example |
|------|---------|-----------|---------|
| `EX_NAME` | Variable reference | text="varname" | `x` |
| `EX_LITERAL` | Literal value | text="42" or text=`"hello"` | `42`, `"str"` |
| `EX_BINARY` | Binary operation | left, text=op, right | `a + b`, `x > y` |
| `EX_UNARY` | Unary operation | text=op, left | `-x`, `!flag` |
| `EX_CALL` | Function call | text="func", args | `add(1, 2)` |
| `EX_INDEX` | Array access | left=array, right=index | `arr[i]` |
| `EX_SLICE` | Slice | left=array, right=start, third=end | `arr[i:j]` |
| `EX_FIELD` | Struct field | left=object, text="field" | `obj.x` |
| `EX_PTR_FIELD` | Pointer field | left=pointer, text="field" | `ptr->x` |
| `EX_INIT_LIST` | Struct init | args=[field_values] | `Point { x: 1, y: 2 }` |
| `EX_TYPE` | Type expression | text="typename" | In `sizeof(i32)` |
| `EX_SIZEOF` | Size operator | left=type_or_expr | `sizeof(i32)` |
| `EX_METHOD_CALL` | Method call | text="method", args | `obj.method()` |
| `EX_QUEUE_METHOD` | Queue/stack op | text="push"/"pop"/etc | `q.push(x)` |

#### Expression Examples

```tig
// Binary operation becomes:
a + b

Expr {
    kind: EX_BINARY,
    left: Expr(EX_NAME, text: "a"),
    text: "+",
    right: Expr(EX_NAME, text: "b")
}
```

```tig
// Function call becomes:
add(1, 2)

Expr {
    kind: EX_CALL,
    text: "add",
    args: [
        Expr(EX_LITERAL, text: "1"),
        Expr(EX_LITERAL, text: "2")
    ]
}
```

### 4. Types (Type)

Type information throughout the program:

```c
struct Type {
    TypeKind kind;           // Type category
    char *name;              // Type name
    Type *inner;             // Inner type (for pointers, etc.)
    Expr *size;              // Array size
    ParamVec params;         // Function parameter types
}
```

#### Type Kinds

| Kind | Purpose | Structure | Example |
|------|---------|-----------|---------|
| `TY_NAME` | Named type | name="i32" | `i32`, `Point` |
| `TY_RAWPTR` | Raw pointer | inner=(pointee type) | `->i32` |
| `TY_FATPTR` | Fat pointer | inner=(element type) | `=>i32` |
| `TY_ARRAY` | Static array | inner=(element), size=10 | `[10]i32` |
| `TY_FNPTR` | Function pointer | inner=(return), params=[...] | `fn i32: i32 x` |
| `TY_QUEUE` | Queue generic | inner=(element type) | `queue<i32>` |
| `TY_STACK` | Stack generic | inner=(element type) | `stack<i32>` |

#### Type Examples

```tig
i32           →  Type { kind: TY_NAME, name: "i32" }
->i32         →  Type { kind: TY_RAWPTR, inner: Type(TY_NAME, "i32") }
=>i32         →  Type { kind: TY_FATPTR, inner: Type(TY_NAME, "i32") }
[10]i32       →  Type { kind: TY_ARRAY, inner: Type(TY_NAME, "i32"), size: Expr(10) }
queue<i32>    →  Type { kind: TY_QUEUE, inner: Type(TY_NAME, "i32") }
```

### 5. Parameters (Param)

Function parameters:

```c
typedef struct {
    Type *type;              // Parameter type
    char *name;              // Parameter name
    bool is_union_field;     // Is union field (struct with &)?
    bool is_anonymous;       // Is anonymous padding (struct)?
} Param;
```

## AST Construction Flow

### 1. Lexer Output
```
Text: "i32 fn add: i32 a, i32 b { ret a + b }"
                ↓
TokenVec: [TOK_KEYWORD(i32), TOK_KEYWORD(fn), TOK_IDENT(add), ...]
```

### 2. Parser Output
```
TokenVec
    ↓
Builds AST:
Decl {
    kind: DC_FN,
    name: "add",
    type: Type(i32),
    params: [Param(i32, "a"), Param(i32, "b")],
    body: [
        Stmt {
            kind: ST_RET,
            expr: Expr(EX_BINARY, a, +, b)
        }
    ]
}
```

### 3. Checker Enhancement
```
AST from Parser
    ↓
Fills in type information:
- expr->type fields are populated
- Variable and function references are validated
- Type compatibility is checked
```

### 4. Emitter Input
```
Fully annotated AST with types
    ↓
Generates C code:
int add(int a, int b) {
  return a + b;
}
```

## Common AST Patterns

### Function with multiple statements
```tig
i32 fn max: i32 a, i32 b {
    if a > b {
        ret a
    }
    ret b
}

→ Decl {
    kind: DC_FN,
    body: [
        Stmt(ST_IF) {
            expr: a > b,
            body: [Stmt(ST_RET, a)],
            else_body: [Stmt(ST_RET, b)]
        }
    ]
}
```

### Struct with methods
```tig
strun Point {
    i32 x
    i32 y
    
    fn void print {
        printi(this.x)
    }
}

→ Decl {
    kind: DC_STRUCT,
    name: "Point",
    params: [Param(i32, "x"), Param(i32, "y")],
    methods: [
        Decl {
            kind: DC_FN,
            name: "print",
            params: [Param(Point, "this")],
            ...
        }
    ]
}
```

### Control flow with multiple branches
```tig
match x {
    1 { printi(1) }
    2 { printi(2) }
    _ { printi(0) }
}

→ Stmt {
    kind: ST_MATCH,
    arms: [
        MatchArm { pattern: 1, body: [...] },
        MatchArm { pattern: 2, body: [...] },
        MatchArm { pattern: NULL, body: [...] }  // _ is NULL
    ]
}
```

## Working with the AST

### Traversing the AST

The checker and emitter both walk the AST:

```c
// Typical traversal pattern
void check_declarations(DeclVec *decls) {
    for (int i = 0; i < decls->count; i++) {
        Decl *decl = decls->items[i];
        switch (decl->kind) {
            case DC_FN: check_function(decl); break;
            case DC_STRUCT: check_struct(decl); break;
            // ... other kinds
        }
    }
}
```

### Creating AST Nodes

Always use the constructor functions:

```c
// Create an expression
Expr *expr = new_expr(EX_BINARY);
expr->left = left_expr;
expr->text = "+";
expr->right = right_expr;

// Create a statement
Stmt *stmt = xmalloc(sizeof(Stmt));
stmt->kind = ST_VAR;
stmt->type = var_type;
stmt->name = "x";
stmt->expr = init_expr;

// Add to vector
stmt_push(&function_body, stmt);
```

## Tips for AST Manipulation

1. **Always check node kind**: Use `switch` on kind enum to handle different node types
2. **Preserve source location**: Keep line/col info in expressions and statements for error reporting
3. **Use vector functions**: Use `expr_push()`, `stmt_push()`, etc. instead of manual append
4. **Initialize fields**: Use `new_expr()`, `new_type()` or `xmalloc` + memset
5. **Handle nested structures**: Remember to recursively process body, params, arms, etc.
6. **Type consistency**: Ensure Type nodes are consistent with what checker validates

## References

- [COMPILER_ARCHITECTURE.md](COMPILER_ARCHITECTURE.md) - Overall compiler design
- [ast.h](../compiler/include/ast.h) - Complete AST type definitions
- [parser.c](../compiler/src/parser.c) - How AST is constructed
- [checker.c](../compiler/src/checker.c) - How AST is validated
- [emitter.c](../compiler/src/emitter.c) - How AST is converted to C

