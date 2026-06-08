<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: cbb12ba6741eaaa4
timestamp: 2026-06-07T21:57:42Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

```markdown
# ast.h - Abstract Syntax Tree Definitions

This module defines the complete Abstract Syntax Tree (AST) structure for the TIG language compiler. The AST represents the program structure after lexical and syntactic analysis, and is used by the checker (semantic analysis) and emitter (code generation) phases.

## Key Data Structures:

- **Type**: Type information (int, ptr, array, function pointer, etc.)
- **Expr**: Expression nodes (literals, operations, function calls, etc.)
- **Stmt**: Statement nodes (assignments, control flow, declarations, etc.)
- **Decl**: Top-level declarations (functions, structs, enums, variables)

These structures form a tree where:
- Decl nodes are at the root level (program)
- Stmt nodes form statement blocks
- Expr nodes form expression trees
- Type nodes annotate declarations and expressions

## Types Defined:

### `TypeKind` Enumeration

Represents all possible type categories in TIG. Used to differentiate types like named types, raw pointers, fat pointers, arrays, function pointers, queues, stacks, etc.

```c
typedef enum {
    TY_NAME,
    TY_RAWPTR,
    TY_FATPTR,
    TY_ARRAY,
    TY_FNPTR,
    TY_QUEUE,
    TY_STACK
} TypeKind;
```

### `Param` Structure

Represents a single parameter in a function signature. Includes fields for type, name (identifier), and whether it's an anonymous padding field or a union field.

```c
typedef struct {
    Type *type;
    char *name;
    bool is_union_field; // For strun: fields prefixed with &.
    bool is_anonymous;   // For strun: anonymous padding fields (no name).
} Param;
```

### `ParamVec`, `ExprVec`, and `StmtVec` Types

Dynamic arrays of parameter, expression, and statement nodes. Used in function signatures, expressions, and statements respectively.

```c
typedef struct {
    Param *items;
    int count;
    int cap;
} ParamVec;

typedef struct {
    Expr **items;
    int count;
    int cap;
} ExprVec;

typedef struct {
    Stmt **items;
    int count;
    int cap;
} StmtVec;
```

### `Type` Structure

Represents any type in TIG programs. The interpretation of fields depends on the TypeKind (TY_NAME, TY_RAWPTR, etc.), and stores relevant information like return type, element type, array dimension, parameter types for function pointers, and queue/stack types.

```c
struct Type {
    TypeKind kind;
    char *name;
    Type *inner;        // return type for TY_FNPTR, pointee for TY_RAWPTR/TY_FATPTR, element for TY_QUEUE/TY_STACK.
    Expr *size;         // array dimension for TY_ARRAY.
    ParamVec params;    // param types for TY_FNPTR
};
```

### `ExprKind` Enumeration

Enumeration of expression types in the AST. Each kind represents a specific type of expression (e.g., variable, literal, binary operation, call to a function).

```c
typedef enum {
    EX_NAME,
    EX_LITERAL,
    EX_BINARY,
    EX_UNARY,
    EX_CALL,
    EX_INDEX,
    EX_SLICE,
    EX_FIELD,
    EX_PTR_FIELD,
    EX_INIT_LIST,
    EX_TYPE,
    EX_VARARGS,
    EX_METHOD_CALL,
    EX_QUEUE_METHOD,
    EX_SIZEOF
} ExprKind;
```

### `Expr` Structure

Represents any expression in the TIG language. Fields include kind, text representation, type annotation, source location (line and column), and operands (left, right, third, args for function calls).

```c
struct Expr {
    ExprKind kind;
    char *text;
    Type *type;
    Expr *left;
    Expr *right;
    Expr *third;
    ExprVec args; // Function arguments for EX_CALL.
    int line;
    int col;
};
```

### `MatchArm` Structure

Represents a single case in a match statement. Includes an expression to match against (or NULL for a wildcard '_') and the statements to execute if this case matches.

```c
typedef struct {
    Expr *pattern; // NULL means wildcard '_'
    StmtVec body;
} MatchArm;
```

### `MatchArmVec` Type

Dynamic array of match arms. Used in match statements to store all case branches.

```c
typedef struct {
    MatchArm *items;
    int count;
    int cap;
} MatchArmVec;
```

### `ElseIfArm` Structure

Represents a single else-if branch in an if statement. Includes the condition expression and the block of statements to execute if this condition is true.

```c
typedef struct {
    Expr *cond;
    StmtVec body;
} ElseIfArm;
```

### `ElseIfVec` Type

Dynamic array of else-if arms. Used in if statements to store all else-if branches.

```c
typedef struct {
    ElseIfArm *items;
    int count;
    int cap;
} ElseIfVec;
```

### `EnumMember` Structure

Represents a single variant in an enum declaration. Includes the name of the variant (e.g., "Option", "Success", "Error").

```c
typedef struct {
    char *name;
} EnumMember;
```

### `EnumMemberVec` Type

Dynamic array of enum variants. Used in enum declarations to store all possible values.

```c
typedef struct {
    EnumMember *items;
    int count;
    int cap;
} EnumMemberVec;
```

### `StmtKind` Enumeration

Enumeration of statement types (ST_VAR, ST_BLOCK, ST_IF, etc.). Each kind represents a specific type of statement in the AST.

```c
typedef enum {
    ST_VAR,
    ST_BLOCK,
    ST_IF,
    ST_LOOP,
    ST_DEFER,
    ST_RET,
    ST_BREAK,
    ST_PIN,
    ST_EXPR,
    ST_MATCH,
    ST_SELECT,
    ST_INLINE_C
} StmtKind;
```

### `Stmt` Structure

Represents any statement in TIG programs. Fields include kind (type of statement), type annotation, variable name, expression, secondary expression, nested statements, match case branches, else-if branches, and final else block.

```c
struct Stmt {
    StmtKind kind;
    Type *type;
    char *name;
    Expr *expr;
    Expr *expr2;
    StmtVec body;
    MatchArmVec arms; // For ST_MATCH.
    ElseIfVec elseifs; // For ST_IF: _if(...){} arms.
    StmtVec else_body; // For ST_IF: _{} final else
    char *text;        // For ST_INLINE_C: the C code text
};
```

### `DeclKind` Enumeration

Enumeration of top-level declarations (DC_USE, DC_STRUCT, DC_ENUM, etc.). Each kind represents a specific type of declaration in the AST.

```c
typedef enum {
    DC_USE,
    DC_STRUCT,
    DC_ENUM,
    DC_FN,
    DC_VAR,
    DC_EXTERN_FN,
    DC_INLINE_C
} DeclKind;
```

### `Decl` Structure

Represents functions, structs, enums, and global variables. The meaning of fields depends on the Declaration kind (DC_USE, DC_STRUCT, etc.).

```c
struct Decl {
    DeclKind kind;
    bool public; // Whether declared with 'pub' keyword.
    bool is_hot;      // For hot reload: function in shared library.
    bool is_async;    // For 1.3: async function.
    char *name;
    char *path;
    Type *type;
    Expr *init;
    ParamVec params;
    StmtVec body;
    bool varargs;
    bool packed;
    EnumMemberVec enum_members; // For DC_ENUM.
    DeclVec methods;             // For DC_STRUCT: methods defined in strun.
    char *text;                  // For DC_INLINE_C: the C code text.
};
```

## Functions for AST Node Creation and Manipulation:

These functions handle memory management and growth of dynamic arrays, including push operations to add new nodes.

```c
/**
 * Vector push functions: Add items to dynamic arrays
 *
 * These functions manage memory allocation and growth of vectors.
 */

void expr_push(ExprVec *v, Expr *x);       // Add expression to vector.
void stmt_push(StmtVec *v, Stmt *x);       // Add statement to vector.
void decl_push(DeclVec *v, Decl *x);       // Add declaration to vector.

/**
 * param_push: Add parameter to parameter vector
 *
 * Args:
 *   v: Target parameter vector.
 *   type: Parameter type.
 *   name: Parameter name (may be NULL).
 *   is_union_field: Whether this is a union field (struct with & prefix).
 *   is_anonymous: Whether this is an anonymous padding field.
 */

void param_push(ParamVec *v, Type *type, char *name, bool is_union_field, bool is_anonymous);

/**
 * match_arm_push: Add case branch to match statement
 *
 * Args:
 *   v: Target match arm vector.
 *   pattern: Expression to match (NULL for wildcard '_').
 *   body: Statements to execute if matched.
 */

void match_arm_push(MatchArmVec *v, Expr *pattern, StmtVec body);

/**
 * elseif_push: Add else-if branch to if statement
 *
 * Args:
 *   v: Target else-if vector.
 *   cond: Condition expression.
 *   body: Statements to execute if condition is true.
 */

void elseif_push(ElseIfVec *v, Expr *cond, StmtVec body);

/**
 * enum_member_push: Add variant to enum declaration
 *
 * Args:
 *   v: Target enum member vector.
 *   name: Name of the variant.
 */

void enum_member_push(EnumMemberVec *v, char *name);
```
```

This documentation provides a comprehensive overview and explanation of each data structure and function defined in the ast.h module.