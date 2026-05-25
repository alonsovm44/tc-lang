#include "ast.h"

#include <string.h>

void expr_push(ExprVec *v, Expr *x) {
    if (v->count == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 8;
        v->items = xrealloc(v->items, sizeof(Expr *) * (size_t)v->cap);
    }
    v->items[v->count++] = x;
}

void stmt_push(StmtVec *v, Stmt *x) {
    if (v->count == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 8;
        v->items = xrealloc(v->items, sizeof(Stmt *) * (size_t)v->cap);
    }
    v->items[v->count++] = x;
}

void decl_push(DeclVec *v, Decl *x) {
    if (v->count == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 8;
        v->items = xrealloc(v->items, sizeof(Decl *) * (size_t)v->cap);
    }
    v->items[v->count++] = x;
}

void param_push(ParamVec *v, Type *type, char *name, bool is_union_field, bool is_anonymous) {
    if (v->count == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 8;
        v->items = xrealloc(v->items, sizeof(Param) * (size_t)v->cap);
    }
    v->items[v->count++] = (Param){type, name, is_union_field, is_anonymous};
}

Type *new_type(TypeKind kind) {
    Type *t = xmalloc(sizeof(Type));
    memset(t, 0, sizeof(Type));
    t->kind = kind;
    return t;
}

Expr *new_expr(ExprKind kind) {
    Expr *e = xmalloc(sizeof(Expr));
    memset(e, 0, sizeof(Expr));
    e->kind = kind;
    return e;
}

Stmt *new_stmt(StmtKind kind) {
    Stmt *s = xmalloc(sizeof(Stmt));
    memset(s, 0, sizeof(Stmt));
    s->kind = kind;
    return s;
}

Decl *new_decl(DeclKind kind) {
    Decl *d = xmalloc(sizeof(Decl));
    memset(d, 0, sizeof(Decl));
    d->kind = kind;
    return d;
}
