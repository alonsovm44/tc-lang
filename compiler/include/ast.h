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
    TY_ARRAY,
    TY_FNPTR,
    TY_QUEUE,
    TY_STACK
} TypeKind;

typedef struct {
    Type *type;
    char *name;
    bool is_union_field;  // For strun: fields prefixed with &
    bool is_anonymous;    // For strun: anonymous padding fields (no name)
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

struct Type {
    TypeKind kind;
    char *name;
    Type *inner;    // return type for TY_FNPTR, pointee for TY_RAWPTR/TY_FATPTR
    Expr *size;
    ParamVec params; // param types for TY_FNPTR
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
    EX_PTR_FIELD,
    EX_INIT_LIST,
    EX_TYPE,
    EX_VARARGS,
    EX_METHOD_CALL,
    EX_QUEUE_METHOD,
    EX_SIZEOF
} ExprKind;

struct Expr {
    ExprKind kind;
    char *text;
    Type *type;
    Expr *left;
    Expr *right;
    Expr *third;
    ExprVec args;
    int line;
    int col;
};

typedef struct {
    Expr *pattern;  // NULL means wildcard '_'
    StmtVec body;
} MatchArm;

typedef struct {
    MatchArm *items;
    int count;
    int cap;
} MatchArmVec;

typedef struct {
    Expr *cond;
    StmtVec body;
} ElseIfArm;

typedef struct {
    ElseIfArm *items;
    int count;
    int cap;
} ElseIfVec;

typedef struct {
    char *name;
} EnumMember;

typedef struct {
    EnumMember *items;
    int count;
    int cap;
} EnumMemberVec;

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
};

typedef enum {
    DC_USE,
    DC_STRUCT,
    DC_ENUM,
    DC_FN,
    DC_VAR,
    DC_EXTERN_FN,
    DC_INLINE_C
} DeclKind;

struct Decl {
    DeclKind kind;
    bool public;
    bool is_hot;  // For hot reload: function is in shared library
    bool is_async;  // For 1.3: async function
    char *name;
    char *path;
    Type *type;
    Expr *init;
    ParamVec params;
    StmtVec body;
    bool varargs;
    bool packed;
    EnumMemberVec enum_members;  // For DC_ENUM
    DeclVec methods;  // For DC_STRUCT: methods defined in strun
    char *text;  // For DC_INLINE_C: the C code text
};

void expr_push(ExprVec *v, Expr *x);
void stmt_push(StmtVec *v, Stmt *x);
void decl_push(DeclVec *v, Decl *x);
void param_push(ParamVec *v, Type *type, char *name, bool is_union_field, bool is_anonymous);
void match_arm_push(MatchArmVec *v, Expr *pattern, StmtVec body);
void elseif_push(ElseIfVec *v, Expr *cond, StmtVec body);
void enum_member_push(EnumMemberVec *v, char *name);
Type *new_type(TypeKind kind);
Expr *new_expr(ExprKind kind);
Stmt *new_stmt(StmtKind kind);
Decl *new_decl(DeclKind kind);

#endif
