/**
 * ast.h - Abstract Syntax Tree Definitions
 *
 * This module defines the complete Abstract Syntax Tree (AST) structure
 * for the TIG language compiler. The AST represents the program structure
 * after lexical and syntactic analysis, and is used by the checker
 * (semantic analysis) and emitter (code generation) phases.
 *
 * Key data structures:
 *  - Type: Type information (int, ptr, array, function pointer, etc.)
 *  - Expr: Expression nodes (literals, operations, function calls, etc.)
 *  - Stmt: Statement nodes (assignments, control flow, declarations, etc.)
 *  - Decl: Top-level declarations (functions, structs, enums, variables)
 *
 * These structures form a tree where:
 *  - Decl nodes are at the root level (program)
 *  - Stmt nodes form statement blocks
 *  - Expr nodes form expression trees
 *  - Type nodes annotate declarations and expressions
 */

#ifndef TIGHTC_AST_H
#define TIGHTC_AST_H

#include "common.h"

typedef struct Type Type;
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Decl Decl;

/**
 * TypeKind: Enumeration of all possible type categories in TIG
 *
 * TY_NAME:   Named type (e.g., i32, i64, u8, struct MyType)
 * TY_RAWPTR: Raw pointer to value type (e.g., ->i32)
 * TY_FATPTR: Fat pointer with length (e.g., =>i32 for slices and arrays)
 * TY_ARRAY:  Static array type (e.g., [10]i32)
 * TY_FNPTR:  Function pointer type (e.g., fn i32: i32 x { ... })
 * TY_QUEUE:  Generic queue type (e.g., queue<i32>)
 * TY_STACK:  Generic stack type (e.g., stack<i32>)
 */
typedef enum {
    TY_NAME,    // Named type: int, struct, enum, etc.
    TY_RAWPTR,  // Raw pointer (->)
    TY_FATPTR,  // Fat pointer with length (=>)
    TY_ARRAY,   // Array type
    TY_FNPTR,   // Function pointer
    TY_QUEUE,   // Generic queue<T>
    TY_STACK    // Generic stack<T>
} TypeKind;

/**
 * Param: Function parameter declaration
 *
 * Represents a single parameter in a function signature.
 *
 * Fields:
 *   type: Type of the parameter
 *   name: Parameter name (identifier)
 *   is_union_field: For struct with union fields (prefixed with & in definition)
 *   is_anonymous: For struct anonymous padding fields (fields without names)
 */
typedef struct {
    Type *type;
    char *name;
    bool is_union_field;  // For strun: fields prefixed with &
    bool is_anonymous;    // For strun: anonymous padding fields (no name)
} Param;

// ============================================================================
// Vector Types: Dynamic arrays for storing AST nodes
// Each vector has: items (array), count (used), cap (capacity)
// ============================================================================

/**
 * ParamVec: Dynamic array of function parameters
 * Used in function declarations and function pointer types
 */
typedef struct {
    Param *items;
    int count;
    int cap;
} ParamVec;

/**
 * ExprVec: Dynamic array of expression nodes
 * Used in function calls (arguments), struct initialization (fields), etc.
 */
typedef struct {
    Expr **items;
    int count;
    int cap;
} ExprVec;

/**
 * StmtVec: Dynamic array of statement nodes
 * Used in function bodies, control flow blocks, etc.
 */
typedef struct {
    Stmt **items;
    int count;
    int cap;
} StmtVec;

/**
 * DeclVec: Dynamic array of declarations
 * Used in the root program AST and method collections
 */
typedef struct {
    Decl **items;
    int count;
    int cap;
} DeclVec;

typedef struct {
    char *name;
    char *replacement;
    char **params;  // For parametric macros
    int param_count;
    bool is_parametric;
} Macro;

typedef struct {
    Macro **items;
    int count;
    int cap;
} MacroVec;

/**
 * Type: Type information in the AST
 *
 * Represents any type in TIG programs. The interpretation of fields
 * depends on the TypeKind:
 *
 * For TY_NAME:     type info for named types (i32, struct, etc)
 *   - name: The type name (e.g., "i32", "MyStruct")
 *   - inner, size, params: unused
 *
 * For TY_RAWPTR:   Raw pointer (->T)
 *   - inner: Points to the base type T
 *   - name, size, params: unused
 *
 * For TY_FATPTR:   Fat pointer with length (=>T)
 *   - inner: Points to the element type T
 *   - name, size, params: unused
 *
 * For TY_ARRAY:    Fixed-size array ([N]T)
 *   - inner: Points to the element type T
 *   - size: Expression for array dimension (e.g., 10)
 *   - name, params: unused
 *
 * For TY_FNPTR:    Function pointer (fn ReturnType: ParamTypes)
 *   - inner: Return type
 *   - params: Parameter types (ParamVec)
 *   - name, size: unused
 *
 * For TY_QUEUE:    Queue<T> generic type
 *   - inner: Element type T
 *   - name, size, params: unused
 *
 * For TY_STACK:    Stack<T> generic type
 *   - inner: Element type T
 *   - name, size, params: unused
 */
struct Type {
    TypeKind kind;
    char *name;
    Type *inner;        // return type for TY_FNPTR, pointee for TY_RAWPTR/TY_FATPTR, element for TY_QUEUE/TY_STACK
    Expr *size;         // array dimension for TY_ARRAY
    ParamVec params;    // param types for TY_FNPTR
};

/**
 * ExprKind: Enumeration of expression types
 *
 * EX_NAME:       Variable/identifier reference (e.g., x, my_var)
 * EX_LITERAL:    Literal value (e.g., 42, "hello", 3.14)
 * EX_BINARY:     Binary operation (e.g., a + b, x * y)
 * EX_UNARY:      Unary operation (e.g., -x, !flag, @ptr)
 * EX_CALL:       Function call (e.g., add(1, 2))
 * EX_INDEX:      Array indexing (e.g., arr[i])
 * EX_SLICE:      Slice operation (e.g., arr[i:j])
 * EX_FIELD:      Struct field access (e.g., obj.field)
 * EX_PTR_FIELD:  Pointer field access (e.g., ptr->field)
 * EX_INIT_LIST:  Struct initialization (e.g., MyStruct { x: 1, y: 2 })
 * EX_TYPE:       Type expression (e.g., sizeof(i32))
 * EX_VARARGS:    Variable arguments (...args in function parameters)
 * EX_METHOD_CALL: Method call on struct (e.g., obj.method())
 * EX_QUEUE_METHOD: Queue/Stack method (e.g., q.push(), q.pop())
 * EX_SIZEOF:     Size-of operator (e.g., sizeof(i32), sizeof(x))
 * EX_CAST:       Type cast (e.g., (f32)x, (->i32)ptr)
 */
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
    EX_SIZEOF,
    EX_CAST
} ExprKind;

/**
 * Expr: Expression node in the AST
 *
 * Represents any expression in the TIG language. The meaning of fields
 * depends on the ExprKind (see specific examples below).
 *
 * Common fields:
 *   kind: What type of expression this is
 *   text: Text representation (variable name, operator, literal value)
 *   type: Inferred/declared type of the expression (set by checker)
 *   line, col: Source location for error reporting
 *
 * Operand fields (used differently by different kinds):
 *   left: Left operand or main operand
 *   right: Right operand
 *   third: Third operand (for slice operations)
 *   args: Function arguments (for calls)
 *
 * Examples:
 *   EX_NAME:     text="x"
 *   EX_LITERAL:  text="42" or text="\"hello\""
 *   EX_BINARY:   left=a, text="+", right=b
 *   EX_CALL:     text="func", args=ArgExpressions
 *   EX_INDEX:    left=arr, right=index
 *   EX_FIELD:    left=obj, text="fieldname"
 */
struct Expr {
    ExprKind kind;
    char *text;
    Type *type;
    Type *cast_type;  // For EX_CAST: target type to cast to
    Expr *left;
    Expr *right;
    Expr *third;
    ExprVec args;
    int line;
    int col;
};

/**
 * MatchArm: A single case in a match statement
 *
 * Fields:
 *   pattern: Expression to match against (NULL for wildcard '_')
 *   body: Statements to execute if pattern matches
 */
typedef struct {
    Expr *pattern;  // NULL means wildcard '_'
    StmtVec body;
} MatchArm;

/**
 * MatchArmVec: Dynamic array of match arms
 * Used in match statements to store all case branches
 */
typedef struct {
    MatchArm *items;
    int count;
    int cap;
} MatchArmVec;

/**
 * ElseIfArm: A single else-if branch in an if statement
 *
 * Fields:
 *   cond: Condition expression to evaluate
 *   body: Statements to execute if condition is true
 */
typedef struct {
    Expr *cond;
    StmtVec body;
} ElseIfArm;

/**
 * ElseIfVec: Dynamic array of else-if arms
 * Used in if statements to store all else-if branches
 */
typedef struct {
    ElseIfArm *items;
    int count;
    int cap;
} ElseIfVec;

/**
 * EnumMember: A single variant in an enum declaration
 *
 * Fields:
 *   name: The variant name (e.g., "Option", "Success", "Error")
 */
typedef struct {
    char *name;
} EnumMember;

/**
 * EnumMemberVec: Dynamic array of enum variants
 * Used in enum declarations to store all possible values
 */
typedef struct {
    EnumMember *items;
    int count;
    int cap;
} EnumMemberVec;

/**
 * CatchArm: A single catch arm in a try-catch block
 *
 * Fields:
 *   error_name: Name of the error type to catch (NULL for wildcard '_')
 *   args: Arguments to match against the error
 *   body: Statements to execute if error matches
 *   ret_expr: Return expression (optional, defaults to 1 or void)
 */
typedef struct {
    char *error_name;  // NULL means wildcard '_'
    ExprVec args;       // Arguments to match
    StmtVec body;       // Statements to execute
    Expr *ret_expr;     // Return expression (optional)
} CatchArm;

/**
 * CatchArmVec: Dynamic array of catch arms
 * Used in try-catch blocks to store all catch branches
 */
typedef struct {
    CatchArm *items;
    int count;
    int cap;
} CatchArmVec;

/**
 * StmtKind: Enumeration of statement types
 *
 * ST_VAR:      Variable declaration (e.g., i32 x = 5)
 * ST_BLOCK:    Block of statements enclosed in { }
 * ST_IF:       If/else-if/else statement
 * ST_LOOP:     Infinite loop with optional break
 * ST_DEFER:    Cleanup statement (executes before function return)
 * ST_RET:      Return statement
 * ST_BREAK:    Break statement (exit current loop)
 * ST_PIN:      Pin statement (mark variable as immutable)
 * ST_EXPR:     Expression statement (expression used as statement)
 * ST_MATCH:    Match/switch statement with pattern matching
 * ST_SELECT:   Select statement (wait for multiple async operations)
 * ST_INLINE_C: Inline C code block
 * ST_THROW:    Throw error statement (e.g., throw MyError(args))
 * ST_TRY:      Try-catch block (e.g., try { ... } catch { ... })
 */
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
    ST_INLINE_C,
    ST_THROW,
    ST_TRY
} StmtKind;

/**
 * Stmt: Statement node in the AST
 *
 * Represents any statement in TIG programs. The meaning of fields
 * depends on the StmtKind.
 *
 * Common fields:
 *   kind: Type of statement
 *   type: Type annotation (for variable declarations)
 *   name: Variable name (for variable declarations)
 *   expr: Primary expression (condition, return value, etc.)
 *   expr2: Secondary expression (for some statements)
 *   body: Block of nested statements
 *   arms: Match case branches
 *   elseifs: Else-if branches
 *   else_body: Final else block
 *   text: C code text (for inline C blocks)
 *   catch_arms: Catch arms for try-catch blocks (ST_TRY)
 *
 * Examples:
 *   ST_VAR:    type=i32, name="x", expr=5
 *   ST_RET:    expr=value_to_return
 *   ST_IF:     expr=condition, body=if_block, elseifs=[], else_body=[]
 *   ST_LOOP:   body=loop_body
 *   ST_DEFER:  body=cleanup_statements
 *   ST_THROW:  expr=error_expression (e.g., MyError(args))
 *   ST_TRY:    body=try_block, catch_arms=[...catch arms...]
 */
struct Stmt {
    StmtKind kind;
    Type *type;
    char *name;
    Expr *expr;
    Expr *expr2;
    StmtVec body;
    MatchArmVec arms;    // For ST_MATCH
    ElseIfVec elseifs;   // For ST_IF: _if(...){} arms
    StmtVec else_body;   // For ST_IF: _{} final else
    char *text;          // For ST_INLINE_C: the C code text
    CatchArmVec catch_arms;  // For ST_TRY: catch arms
};

/**
 * DeclKind: Enumeration of top-level declarations
 *
 * DC_USE:        Import statement (e.g., use "lib.tc")
 * DC_STRUCT:     Struct type definition (includes union fields and methods)
 * DC_ENUM:       Enum type definition
 * DC_ERROR:      Error type definition (e.g., error MyError: i32 code { ... })
 * DC_FN:         Function definition
 * DC_VAR:        Global variable declaration
 * DC_EXTERN_FN:  External C function declaration
 * DC_INLINE_C:   Inline C code at module level
 */
typedef enum {
    DC_USE,
    DC_STRUCT,
    DC_ENUM,
    DC_ERROR,
    DC_FN,
    DC_VAR,
    DC_EXTERN_FN,
    DC_INLINE_C
} DeclKind;

/**
 * Decl: Top-level declaration node in the AST
 *
 * Represents functions, structs, enums, and global variables.
 * The meaning of fields depends on the DeclKind.
 *
 * Common fields:
 *   kind: Type of declaration
 *   name: Declaration name (function, struct, variable)
 *   type: Type information (return type for functions, etc.)
 *   is_hot: Whether this is in a hot-reload library
 *   is_async: Whether this is an async function
 *
 * Function-specific fields (DC_FN):
 *   params: Function parameters
 *   body: Function statements
 *   varargs: Whether function accepts variable arguments
 *
 * Struct-specific fields (DC_STRUCT):
 *   params: Struct fields (stored as parameters)
 *   packed: Whether struct is packed (no padding)
 *   methods: Methods defined on this struct
 *
 * Enum-specific fields (DC_ENUM):
 *   enum_members: List of enum variants
 *
 * Import-specific fields (DC_USE):
 *   path: Path to imported file
 *
 * Global variable-specific fields (DC_VAR):
 *   init: Initialization expression
 *
 * Inline C-specific fields (DC_INLINE_C):
 *   text: Raw C code
 *
 * Examples:
 *   DC_FN:    name="add", type=i32, params=[i32 a, i32 b], body={...}
 *   DC_STRUCT: name="Point", params=[i32 x, i32 y], packed=false
 *   DC_ENUM:  name="Option", enum_members=["Some", "None"]
 */
struct Decl {
    DeclKind kind;
    bool is_hot;     // For hot reload: function is in shared library
    bool is_async;   // For 1.3: async function
    bool is_my;      // For 1.3.1: marked as private with 'my' keyword
    bool is_raw;     // For OSdev: function accesses memory directly (volatile)
    char *name;
    char *path;
    char *source_file; // Source file where this declaration was defined
    Type *type;
    Expr *init;
    ParamVec params;
    StmtVec body;
    bool varargs;
    bool packed;
    EnumMemberVec enum_members;  // For DC_ENUM
    DeclVec methods;             // For DC_STRUCT: methods defined in strun
    char *text;                  // For DC_INLINE_C: the C code text
};

// ============================================================================
// AST Node Creation and Manipulation Functions
// ============================================================================

/**
 * Vector push functions: Add items to dynamic arrays
 * These functions handle memory management and growth of vectors.
 */
void expr_push(ExprVec *v, Expr *x);       // Add expression to vector
void stmt_push(StmtVec *v, Stmt *x);       // Add statement to vector
void decl_push(DeclVec *v, Decl *x);       // Add declaration to vector

/**
 * param_push: Add parameter to parameter vector
 * Args:
 *   v: Target parameter vector
 *   type: Parameter type
 *   name: Parameter name
 *   is_union_field: Whether this is a union field (struct with & prefix)
 *   is_anonymous: Whether this is an anonymous padding field
 */
void param_push(ParamVec *v, Type *type, char *name, bool is_union_field, bool is_anonymous);

/**
 * match_arm_push: Add case branch to match statement
 * Args:
 *   v: Target match arm vector
 *   pattern: Expression to match (NULL for wildcard '_')
 *   body: Statements to execute if matched
 */
void match_arm_push(MatchArmVec *v, Expr *pattern, StmtVec body);

/**
 * elseif_push: Add else-if branch to if statement
 * Args:
 *   v: Target else-if vector
 *   cond: Condition expression
 *   body: Statements to execute if condition is true
 */
void elseif_push(ElseIfVec *v, Expr *cond, StmtVec body);

/**
 * enum_member_push: Add variant to enum declaration
 * Args:
 *   v: Target enum member vector
 *   name: Name of the variant
 */
void enum_member_push(EnumMemberVec *v, char *name);

/**
 * catch_arm_push: Add catch arm to try-catch block
 * Args:
 *   v: Target catch arm vector
 *   error_name: Name of error type to catch (NULL for wildcard '_')
 *   args: Arguments to match against the error
 *   body: Statements to execute if matched
 *   ret_expr: Return expression (optional)
 */
void catch_arm_push(CatchArmVec *v, char *error_name, ExprVec args, StmtVec body, Expr *ret_expr);

/**
 * new_type: Create a new type node
 * Args:
 *   kind: Type kind (TY_NAME, TY_RAWPTR, etc.)
 * Returns:
 *   Newly allocated Type node with fields zero-initialized
 */
Type *new_type(TypeKind kind);

/**
 * new_expr: Create a new expression node
 * Args:
 *   kind: Expression kind (EX_BINARY, EX_CALL, etc.)
 * Returns:
 *   Newly allocated Expr node with fields zero-initialized
 */
Expr *new_expr(ExprKind kind);
Stmt *new_stmt(StmtKind kind);
Decl *new_decl(DeclKind kind);

#endif
