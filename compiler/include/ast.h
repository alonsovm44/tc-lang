#ifndef TIGHTC_AST_H
#define TIGHTC_AST_H

#include "common.h"

typedef struct Type Type;
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Decl Decl;

typedef enum {
    TY_NAME,
    TY_RAWPTR,
    TY_FATPTR,
    TY_ARRAY
} TypeKind;

typedef struct {
    Type *type;
    char *name;
} Param;

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

typedef struct {
    Decl **items;
    int count;
    int cap;
} DeclVec;

struct Type {
    TypeKind kind;
    char *name;
    Type *inner;
    Expr *size;
};

typedef enum {
    EX_NAME,
    EX_LITERAL,
    EX_BINARY,
    EX_UNARY,
    EX_CALL,
    EX_INDEX,
    EX_SLICE,
    EX_FIELD,
    EX_INIT_LIST,
    EX_TYPE
} ExprKind;

struct Expr {
    ExprKind kind;
    char *text;
    Type *type;
    Expr *left;
    Expr *right;
    Expr *third;
    ExprVec args;
};

typedef enum {
    ST_VAR,
    ST_BLOCK,
    ST_IF,
    ST_LOOP,
    ST_DEFER,
    ST_RET,
    ST_BREAK,
    ST_PIN,
    ST_EXPR
} StmtKind;

struct Stmt {
    StmtKind kind;
    Type *type;
    char *name;
    Expr *expr;
    Expr *expr2;
    StmtVec body;
};

typedef enum {
    DC_USE,
    DC_STRUCT,
    DC_FN,
    DC_VAR,
    DC_EXTERN_FN
} DeclKind;

struct Decl {
    DeclKind kind;
    bool public;
    char *name;
    char *path;
    Type *type;
    Expr *init;
    ParamVec params;
    StmtVec body;
    bool varargs;
};

void expr_push(ExprVec *v, Expr *x);
void stmt_push(StmtVec *v, Stmt *x);
void decl_push(DeclVec *v, Decl *x);
void param_push(ParamVec *v, Type *type, char *name);
Type *new_type(TypeKind kind);
Expr *new_expr(ExprKind kind);
Stmt *new_stmt(StmtKind kind);
Decl *new_decl(DeclKind kind);

#endif
