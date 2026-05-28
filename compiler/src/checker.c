#include "checker.h"
#include "common.h"
#include "ast.h"

#include <stdlib.h>
#include <string.h>

// Scope stack entry
typedef struct {
    char **vars;
    int count;
    int cap;
} Scope;

typedef struct {
    Scope *scopes;
    int depth;
    int cap;
    DeclVec *program;
} ScopeStack;

static void push_scope(ScopeStack *s) {
    if (s->depth == s->cap) {
        s->cap = s->cap ? s->cap * 2 : 8;
        s->scopes = xrealloc(s->scopes, sizeof(Scope) * (size_t)s->cap);
    }
    s->scopes[s->depth++] = (Scope){0};
}

static void pop_scope(ScopeStack *s) {
    if (s->depth == 0) return;
    s->depth--;
    // free the vars array in the popped scope
    if (s->scopes[s->depth].vars) {
        free(s->scopes[s->depth].vars);
        s->scopes[s->depth].vars = NULL;
        s->scopes[s->depth].count = 0;
        s->scopes[s->depth].cap = 0;
    }
}

static void declare_var(ScopeStack *s, const char *name) {
    Scope *top = &s->scopes[s->depth - 1];
    if (top->count == top->cap) {
        top->cap = top->cap ? top->cap * 2 : 8;
        top->vars = xrealloc(top->vars, sizeof(char *) * (size_t)top->cap);
    }
    top->vars[top->count++] = (char *)name;
}

static bool var_in_scope(ScopeStack *s, const char *name) {
    for (int i = s->depth - 1; i >= 0; i--) {
        for (int j = 0; j < s->scopes[i].count; j++) {
            if (strcmp(s->scopes[i].vars[j], name) == 0) return true;
        }
    }
    return false;
}

static bool fn_exists(DeclVec *program, const char *name) {
    for (int i = 0; i < program->count; i++) {
        Decl *d = program->items[i];
        if ((d->kind == DC_FN || d->kind == DC_EXTERN_FN) && strcmp(d->name, name) == 0) return true;
    }
    return false;
}

static void check_expr(Expr *e, ScopeStack *s);

static void check_expr(Expr *e, ScopeStack *s) {
    if (!e) return;
    switch (e->kind) {
        case EX_NAME:
            if (!var_in_scope(s, e->text) && !fn_exists(s->program, e->text)) {
                tc_error("E014", e->line, e->col, (int)strlen(e->text),
                    "undefined variable '%s'", e->text);
            }
            break;
        case EX_CALL:
            for (int i = 0; i < e->args.count; i++) check_expr(e->args.items[i], s);
            break;
        case EX_BINARY:
            check_expr(e->left, s);
            check_expr(e->right, s);
            break;
        case EX_UNARY:
            check_expr(e->left, s);
            break;
        case EX_INDEX:
            check_expr(e->left, s);
            check_expr(e->right, s);
            break;
        case EX_FIELD:
            check_expr(e->left, s);
            break;
        case EX_SLICE:
            check_expr(e->left, s);
            check_expr(e->right, s);
            check_expr(e->third, s);
            break;
        case EX_INIT_LIST:
            for (int i = 0; i < e->args.count; i++) check_expr(e->args.items[i], s);
            break;
        case EX_LITERAL:
        case EX_TYPE:
            break;
    }
}

static void check_stmts(StmtVec *body, ScopeStack *s);

static void check_stmts(StmtVec *body, ScopeStack *s) {
    for (int i = 0; i < body->count; i++) {
        Stmt *st = body->items[i];
        switch (st->kind) {
            case ST_VAR:
                if (st->expr) check_expr(st->expr, s);
                declare_var(s, st->name);
                break;
            case ST_EXPR:
                check_expr(st->expr, s);
                break;
            case ST_IF:
                check_expr(st->expr, s);
                push_scope(s); check_stmts(&st->body, s); pop_scope(s);
                for (int j = 0; j < st->elseifs.count; j++) {
                    check_expr(st->elseifs.items[j].cond, s);
                    push_scope(s); check_stmts(&st->elseifs.items[j].body, s); pop_scope(s);
                }
                if (st->else_body.count) {
                    push_scope(s); check_stmts(&st->else_body, s); pop_scope(s);
                }
                break;
            case ST_LOOP:
                if (st->expr) check_expr(st->expr, s);
                push_scope(s);
                check_stmts(&st->body, s);
                pop_scope(s);
                break;
            case ST_BLOCK:
                push_scope(s);
                check_stmts(&st->body, s);
                pop_scope(s);
                break;
            case ST_DEFER:
                push_scope(s);
                check_stmts(&st->body, s);
                pop_scope(s);
                break;
            case ST_MATCH:
                check_expr(st->expr, s);
                for (int j = 0; j < st->arms.count; j++) {
                    push_scope(s);
                    check_stmts(&st->arms.items[j].body, s);
                    pop_scope(s);
                }
                break;
            case ST_RET:
                check_expr(st->expr, s);
                break;
            case ST_BREAK:
            case ST_PIN:
                break;
        }
    }
}

void check_program(DeclVec *program) {
    for (int i = 0; i < program->count; i++) {
        Decl *d = program->items[i];
        if (d->kind != DC_FN) continue;

        ScopeStack s = {0};
        s.program = program;

        push_scope(&s);
        // Declare function parameters in scope
        for (int j = 0; j < d->params.count; j++) {
            declare_var(&s, d->params.items[j].name);
        }
        check_stmts(&d->body, &s);
        pop_scope(&s);

        if (s.scopes) free(s.scopes);
    }
}
