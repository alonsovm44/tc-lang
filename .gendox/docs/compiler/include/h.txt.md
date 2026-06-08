<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: 7bda34ead9a75ee9
timestamp: 2026-06-07T22:03:57Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

## Ast (Abstract Syntax Tree)

### Purpose and Usage

The `Ast` module is designed to represent the Abstract Syntax Tree (AST) of a TightC program. It consists of various data structures for different elements such as types, expressions, statements, and declarations. The primary goal is to provide a structured representation that can be used by the compiler's parser and other tools.

### Types

- **Type**: Represents the structure of a type definition in the program.
  - `type`: Pointer to a child node representing its inner type or pointer.
  - `name`: Name of the type (e.g., "i32", "fn").
  - `inner`: Points to another type if it's a raw, fat, or function pointer.
  - `size`: A pointer to an expression indicating the size.
  - `params`: Vector of parameter types for function pointers.

- **Expr**: Represents expressions within the AST. Each node can have different kinds such as literal, binary, unary, call, etc.
  - `kind`: Kind of the expression (e.g., TOK_NUMBER).
  - `text`: The text representation of the expression.
  - `type`: Pointer to a type associated with this expression.
  - `left`, `right`, `third`: Pointers to other expressions forming the structure of binary and ternary operations.
  - `args`: Vector of argument expressions for functions.

- **Stmt**: Represents statements within the AST. This includes variables, blocks, loops, function definitions, etc.
  - `kind`: Kind of the statement (e.g., TOK_IF).
  - `type`, `name`, `expr`, `expr2`: Pointers to associated types or values.
  - `body`: Vector of other statements forming a block structure.
  - `arms`: Vector of arms for match statements.

- **Decl**: Represents declarations within the AST. This includes functions, variables, unions, etc.
  - `kind`: Kind of declaration (e.g., TOK_VAR).
  - `public`, `name`, `path`, `type`, `init`, `params`, `body`, `varargs`, `packed`: Various fields representing attributes and associated nodes.

### Functions

- **expr_push**: Pushes an expression onto a vector.
- **stmt_push**: Pushes a statement onto a vector.
- **decl_push**: Pushes a declaration onto a vector.
- **param_push**: Adds a parameter to the parameter vector.
- **match_arm_push**: Adds an arm for match statements.
- **new_type**: Creates a new type with specified kind, name, inner pointer, and size pointer.
- **new_expr**: Creates a new expression with specified kind, text, type, and children pointers.
- **new_stmt**: Creates a new statement with specified kind, type, name, expression, second expression, and body vector.
- **new_decl**: Creates a new declaration with specified kind, public/private visibility, name/path, type, initializer, parameter vector, body vector, varargs flag, and packed flag.

### Token Kind

- **TOK_EOF**: End of input marker.
- **TOK_IDENT**: Identifier (variable/function name).
- **TOK_NUMBER**: Numeric literal (integer or floating point).
- **TOK_STRING**: String literal (including quotes).
- **TOK_SYMBOL**: Operator or delimiter (single or double char).
- **TOK_KEYWORD**: Reserved keyword (if, loop, fn, ret, etc.).

### TokenVec

- A structure containing a vector of tokens with fields for the token array, count, and capacity.

### Lexing Function

- **lex_source**: Transforms input source code into a stream of tokens using lexical analysis. The function handles various aspects such as keywords, identifiers, operators, literals, etc.
  - **Args**:
    - `src`: Input source code as a null-terminated string.
  - **Returns**:
    - A vector containing all the tokens from the input.

### Notes

This module provides a foundational structure for constructing and manipulating an AST. The functions defined here help in creating, pushing onto vectors, and initializing nodes of various types within the tree. Additionally, it includes utility functions like error handling (`die`), string manipulation (`xmalloc`, `xrealloc`, etc.), source context setting (`tc_set_source`), and error reporting (`tc_error`).