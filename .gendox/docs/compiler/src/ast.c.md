<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: 27fe31dd255126e1
timestamp: 2026-06-07T22:10:29Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# Ast.h Documentation

## Namespace: ast

### Files that use this header:
No specific files are mentioned in the provided code. This documentation will focus on explaining the purpose and usage of each function within the `ast` namespace, as they seem to be part of an Abstract Syntax Tree (AST) implementation.

---

## 1. expr_push(ExprVec *v, Expr *x)

### Description:
This function pushes a new element onto the end of the list stored in `v`. It ensures that memory allocation is doubled each time it runs into capacity issues until the allocated memory doubles from its initial size to at least 8.

### Parameters:
- **v** (ExprVec \*): A pointer to an ExprVec structure, which contains a linked list of expressions.
- **x** (Expr \*): A pointer to the expression (`Expr`) that needs to be added to `v`.

### Example Usage:
```c
ExprVec exprs;
// Initialize and push some expressions...
expr_push(&exprs, new_expr(EXPR_KIND));
```

---

## 2. stmt_push(StmtVec *v, Stmt *x)

### Description:
Similar to `expr_push`, this function adds a new element (`Stmt`) onto the end of the list in `v`. It doubles the capacity of the memory allocation whenever it reaches its maximum limit until doubling from the initial size.

### Parameters:
- **v** (StmtVec \*): A pointer to an StmtVec structure, which holds a linked list of statements.
- **x** (Stmt \*): A pointer to the statement (`Stmt`) that needs to be added to `v`.

### Example Usage:
```c
StmtVec stmts;
// Initialize and push some statements...
stmt_push(&stmts, new_stmt(STMK_KIND));
```

---

## 3. decl_push(DeclVec *v, Decl *x)

### Description:
This function is used similarly to the previous two functions but applies to declarations (`Decl`), which are fundamental components of an AST.

### Parameters:
- **v** (DeclVec \*): A pointer to a DeclVec structure that stores linked lists of declarations.
- **x** (Decl \*): A pointer to a declaration (`Decl`) to be added to `v`.

### Example Usage:
```c
DeclVec decls;
// Initialize and push some declarations...
decl_push(&decls, new_decl(DLKIND));
```

---

## 4. param_push(ParamVec *v, Type *type, char *name, bool is_union_field, bool is_anonymous)

### Description:
This function inserts a parameter (`Param`) into the `ParamVec` structure's list of parameters. It ensures that memory allocation doubles every time its capacity limit is reached until doubling from its initial size.

### Parameters:
- **v** (ParamVec \*): A pointer to a ParamVec, which stores linked lists of parameters.
- **type** (Type \*) : A pointer to the type associated with this parameter.
- **name** (char \*) : The name assigned to this parameter.
- **is_union_field** (bool) : Indicates whether the parameter is part of a union field.
- **is_anonymous** (bool) : Indicates if the parameter has an anonymous declaration.

### Example Usage:
```c
ParamVec params;
// Initialize and push parameters...
param_push(&params, new_type(TYPER), "name", false, true);
```

---

## 5. match_arm_push(MatchArmVec *v, Expr *pattern, StmtVec body)

### Description:
This function appends a `MatchArm` into the list of arms in the `MatchArmVec`. It also requires two parameters: one for the pattern (`Expr`) and another for the corresponding action (body).

### Parameters:
- **v** (MatchArmVec \*): A pointer to a MatchArmVec, which holds linked lists of match arms.
- **pattern** (Expr \*) : The pattern used in matching expressions.
- **body** (StmtVec \*) : Linked list containing the body statements corresponding to each arm.

### Example Usage:
```c
MatchArmVec arms;
// Initialize and push match arms...
match_arm_push(&arms, new_expr(EXPR), stmts);
```

---

## 6. elseif_push(ElseIfVec *v, Expr *cond, StmtVec body)

### Description:
This function adds an `ElseIf` arm to the vector of else-if conditions (`ElseIfVec`). It requires a condition expression and a corresponding body statement list.

### Parameters:
- **v** (ElseIfVec \*): A pointer to a ElseIfVec structure that stores linked lists of ElseIf arms.
- **cond** (Expr \*) : The condition for the `ElseIf` arm.
- **body** (StmtVec \*) : Linked list containing statements under this condition.

### Example Usage:
```c
ElseIfVec elifs;
// Initialize and push else-if arms...
elseif_push(&elifs, new_expr(EXPR), stmts);
```

---

## 7. enum_member_push(EnumMemberVec *v, char *name)

### Description:
This function is used to add a member to an enumeration (`Enum`). It needs the name of the member.

### Parameters:
- **v** (EnumMemberVec \*): A pointer to an EnumMemberVec structure that holds linked lists of enum members.
- **name** (char \*) : The name of the new enum member.

### Example Usage:
```c
EnumMemberVec enums;
// Initialize and push enum members...
enum_member_push(&enums, "member_name");
```

---

## 8. Type *new_type(TypeKind kind)

### Description:
This function allocates memory for a `Type` structure according to the given type kind (`kind`). It initializes the member variables of this structure.

### Parameters:
- **kind** (TypeKind): The type kind identifier (e.g., `TYP_INT`, `TYP_STRING`) that indicates the type of data the struct will hold.

### Example Usage:
```c
Type *int_type = new_type(TYPER_INT);
```

---

## 9. Expr *new_expr(ExprKind kind)

### Description:
This function allocates memory for an `Expr` structure based on the given expression kind (`kind`). It initializes this struct's members with default values.

### Parameters:
- **kind** (ExprKind): The kind of expression (e.g., `EXPR_ADD`, `EXPR_SUB`) to be represented by the new `Expr`.

### Example Usage:
```c
Expr *add_expr = new_expr(EXPR_ADD);
```

---

## 10. Stmt *new_stmt(StmtKind kind)

### Description:
This function allocates memory for a `Stmt` structure based on the given statement kind (`kind`). It initializes this struct's members with default values.

### Parameters:
- **kind** (StmtKind): The type of statement to be represented by the new `Stmt`.

### Example Usage:
```c
Stmt *assign_stmt = new_stmt(STMK_ASSIGN);
```

---

## 11. Decl *new_decl(DeclKind kind)

### Description:
This function allocates memory for a `Decl` structure based on the given declaration kind (`kind`). It initializes this struct's members with default values.

### Parameters:
- **kind** (DeclKind): The type of declaration to be represented by the new `Decl`.

### Example Usage:
```c
Decl *var_decl = new_decl(DLK_VAR);
```

---

## Notes

In the provided code, there is no explicit documentation for functions such as `xmalloc`, `xrealloc`. These appear to be utility functions used internally. The primary focus here was on explaining the use and functionality of each specific function related to an AST implementation.