#include "checker.h"
#include "common.h"
#include "ast.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Variable information for scope tracking
typedef struct {
    char *name;
    Type *type;
    bool is_owned;     // Variable ownership status
    bool is_dead;       // Variable is dead (ownership transferred)
    bool is_pinned;     // Variable is pinned (immutable)
} VarInfo;

// Scope stack entry
typedef struct {
    VarInfo *vars;
    int count;
    int cap;
} Scope;

typedef struct {
    Scope *scopes;
    int depth;
    int cap;
    DeclVec *program;
    Decl *current_fn;  // Current function being checked
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

static void declare_var(ScopeStack *s, const char *name, Type *type) {
    Scope *top = &s->scopes[s->depth - 1];
    if (top->count == top->cap) {
        top->cap = top->cap ? top->cap * 2 : 8;
        top->vars = xrealloc(top->vars, sizeof(VarInfo) * (size_t)top->cap);
    }
    // Determine if this is an owned type (pointer)
    bool is_owned = (type && (type->kind == TY_RAWPTR || type->kind == TY_FATPTR));
    top->vars[top->count++] = (VarInfo){(char *)name, type, is_owned, false, false};
}

static VarInfo *get_var_info(ScopeStack *s, const char *name) {
    for (int i = s->depth - 1; i >= 0; i--) {
        for (int j = 0; j < s->scopes[i].count; j++) {
            if (strcmp(s->scopes[i].vars[j].name, name) == 0) {
                return &s->scopes[i].vars[j];
            }
        }
    }
    return NULL;
}

static void mark_var_dead(ScopeStack *s, const char *name) {
    VarInfo *var = get_var_info(s, name);
    if (var) {
        var->is_dead = true;
    }
}

static void check_var_access(ScopeStack *s, const char *name, int line, int col) {
    VarInfo *var = get_var_info(s, name);
    if (var && var->is_dead) {
        tc_error("E010", line, col, (int)strlen(name), 
            "Cannot access dead variable '%s'", name);
    }
}

static bool var_in_scope(ScopeStack *s, const char *name) {
    for (int i = s->depth - 1; i >= 0; i--) {
        for (int j = 0; j < s->scopes[i].count; j++) {
            if (strcmp(s->scopes[i].vars[j].name, name) == 0) return true;
        }
    }
    return false;
}

static Type *get_var_type(ScopeStack *s, const char *name) {
    for (int i = s->depth - 1; i >= 0; i--) {
        for (int j = 0; j < s->scopes[i].count; j++) {
            if (strcmp(s->scopes[i].vars[j].name, name) == 0) {
                return s->scopes[i].vars[j].type;
            }
        }
    }
    return NULL;
}

static bool fn_exists(DeclVec *program, const char *name) {
    for (int i = 0; i < program->count; i++) {
        Decl *d = program->items[i];
        if ((d->kind == DC_FN || d->kind == DC_EXTERN_FN) && strcmp(d->name, name) == 0) return true;
    }
    return false;
}

static Decl *get_fn_decl(DeclVec *program, const char *name) {
    for (int i = 0; i < program->count; i++) {
        Decl *d = program->items[i];
        if ((d->kind == DC_FN || d->kind == DC_EXTERN_FN) && strcmp(d->name, name) == 0) return d;
    }
    return NULL;
}

static bool is_assignment(const char *op) {
    return !strcmp(op, "=") || !strcmp(op, "+=") || !strcmp(op, "-=") || 
           !strcmp(op, "*=") || !strcmp(op, "/=") || !strcmp(op, "%=");
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
            // Check for dead variable access
            check_var_access(s, e->text, e->line, e->col);
            // Set type for variable expressions by looking up the variable
            e->type = get_var_type(s, e->text);
            break;
        case EX_CALL:
            for (int i = 0; i < e->args.count; i++) check_expr(e->args.items[i], s);
            // Check argument count against function signature
            if (e->text && fn_exists(s->program, e->text)) {
                Decl *fn = get_fn_decl(s->program, e->text);
                if (fn) {
                    int expected_params = fn->params.count;
                    int actual_args = e->args.count;
                    if (!fn->varargs && actual_args != expected_params) {
                        if (actual_args < expected_params) {
                            tc_error("E015", e->line, e->col, (int)strlen(e->text),
                                "Too few arguments to function '%s': expected %d, got %d", 
                                e->text, expected_params, actual_args);
                        } else {
                            tc_error("E016", e->line, e->col, (int)strlen(e->text),
                                "Too many arguments to function '%s': expected %d, got %d", 
                                e->text, expected_params, actual_args);
                        }
                    }
                    // Check that owned arguments are passed with @ operator (only for async functions)
                    if (fn->is_async) {
                        for (int i = 0; i < e->args.count && i < fn->params.count; i++) {
                            Expr *arg = e->args.items[i];
                            Type *param_type = fn->params.items[i].type;
                            // If parameter expects an owned type (pointer)
                            if (param_type && (param_type->kind == TY_RAWPTR || param_type->kind == TY_FATPTR)) {
                                // Check if argument is a variable name without @
                                if (arg->kind == EX_NAME) {
                                    VarInfo *var = get_var_info(s, arg->text);
                                    if (var && var->is_owned) {
                                        // Variable is owned but not marked with @
                                        tc_error("E017", e->line, e->col, (int)strlen(arg->text),
                                            "Owned variable '%s' must be passed with @ operator", arg->text);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        case EX_BINARY:
            check_expr(e->left, s);
            check_expr(e->right, s);
            // Check for assignment to pinned variable
            if (is_assignment(e->text) && e->left->kind == EX_NAME) {
                VarInfo *var = get_var_info(s, e->left->text);
                if (var && var->is_pinned) {
                    tc_error("E013", e->line, e->col, (int)strlen(e->left->text),
                        "Cannot assign to pinned variable '%s'", e->left->text);
                }
            }
            break;
        case EX_UNARY:
            check_expr(e->left, s);
            if (!strcmp(e->text, "&ref") || !strcmp(e->text, "&")) {
                e->type = new_type(TY_RAWPTR);
                e->type->inner = e->left->type;
            }
            // Handle ownership transfer with @ operator
            if (strcmp(e->text, "@") == 0 && e->left->kind == EX_NAME) {
                VarInfo *var = get_var_info(s, e->left->text);
                if (var) {
                    if (!var->is_owned) {
                        tc_error("E011", e->line, e->col, (int)strlen(e->left->text),
                            "Cannot give owned variable without @ - '%s' is not an owned type", e->left->text);
                    }
                    if (var->is_dead) {
                        tc_error("E010", e->line, e->col, (int)strlen(e->left->text),
                            "Cannot access dead variable '%s'", e->left->text);
                    }
                    // Mark variable as dead after ownership transfer
                    mark_var_dead(s, e->left->text);
                }
            }
            break;
        case EX_INDEX:
            check_expr(e->left, s);
            check_expr(e->right, s);
            break;
        case EX_METHOD_CALL:
            check_expr(e->left, s);
            for (int i = 0; i < e->args.count; i++) check_expr(e->args.items[i], s);
            if (e->left->type && (e->left->type->kind == TY_QUEUE || e->left->type->kind == TY_STACK) &&
                (!strcmp(e->text, "push") || !strcmp(e->text, "enq") || !strcmp(e->text, "pop") || !strcmp(e->text, "deq") || !strcmp(e->text, "peek"))) {
                e->kind = EX_QUEUE_METHOD;
                if (!strcmp(e->text, "pop") || !strcmp(e->text, "deq") || !strcmp(e->text, "peek")) {
                    if (e->left->type && e->left->type->inner) {
                        e->type = e->left->type->inner;
                    }
                } else {
                    e->type = new_type(TY_NAME);
                    e->type->name = xstrdup("void");
                }
            } else if (e->left->type) {
                e->type = e->left->type;
            }
            break;
        case EX_QUEUE_METHOD:
            check_expr(e->left, s);
            // For push/pop methods, set appropriate return type
            if (strcmp(e->text, "pop") == 0) {
                // pop() returns the inner type of the queue/stack
                if (e->left->type && e->left->type->inner) {
                    e->type = e->left->type->inner;
                }
            } else {
                // push() returns void
                e->type = new_type(TY_NAME);
                e->type->name = xstrdup("void");
            }
            for (int i = 0; i < e->args.count; i++) check_expr(e->args.items[i], s);
            break;
        case EX_FIELD:
        case EX_PTR_FIELD:
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
        case EX_VARARGS:
            break;
        case EX_SIZEOF:
            // sizeof always returns i32 (size in bytes)
            e->type = new_type(TY_NAME);
            e->type->name = xstrdup("i32");
            // Check the type expression stored in left field
            if (e->left) check_expr(e->left, s);
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
                declare_var(s, st->name, st->type);
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
                // Check if we're in an async function and returning a value
                if (s->current_fn && s->current_fn->is_async && st->expr) {
                    tc_error("E012", 0, 0, 4, 
                        "Async functions cannot return values");
                }
                break;
            case ST_SELECT:
                // TODO: Implement select statement checking
                break;
            case ST_BREAK:
                break;
            case ST_PIN:
                // Pin statement: pin variable
                if (st->expr && st->expr->kind == EX_NAME) {
                    VarInfo *var = get_var_info(s, st->expr->text);
                    if (var) {
                        var->is_pinned = true;
                    }
                }
                check_expr(st->expr, s);
                break;
            case ST_INLINE_C:
                break;
            case ST_THROW:
                check_expr(st->expr, s);
                break;
            case ST_TRY:
                push_scope(s);
                check_stmts(&st->body, s);
                pop_scope(s);
                // Check catch arms
                for (int j = 0; j < st->catch_arms.count; j++) {
                    CatchArm *arm = &st->catch_arms.items[j];
                    // Check arguments in catch pattern
                    for (int k = 0; k < arm->args.count; k++) {
                        check_expr(arm->args.items[k], s);
                    }
                    // Check return expression
                    if (arm->ret_expr) {
                        check_expr(arm->ret_expr, s);
                    }
                    // Check catch body
                    if (arm->body.count) {
                        push_scope(s);
                        check_stmts(&arm->body, s);
                        pop_scope(s);
                    }
                }
                break;
        }
    }
}

void check_program(DeclVec *program) {
    // Check for method/field name conflicts in structs
    for (int i = 0; i < program->count; i++) {
        Decl *d = program->items[i];
        if (d->kind == DC_STRUCT) {
            // Check each method against each field
            for (int m = 0; m < d->methods.count; m++) {
                Decl *method = d->methods.items[m];
                for (int f = 0; f < d->params.count; f++) {
                    Param *field = &d->params.items[f];
                    if (strcmp(method->name, field->name) == 0) {
                        fprintf(stderr, "error: method '%s' has the same name as a field in struct '%s'\n", method->name, d->name);
                        exit(1);
                    }
                }
            }
        }
    }

    for (int i = 0; i < program->count; i++) {
        Decl *d = program->items[i];
        if (d->kind != DC_FN) {
            // Skip inline C declarations - they don't need type checking
            if (d->kind == DC_INLINE_C) continue;
            continue;
        }

        ScopeStack s = {0};
        s.program = program;
        s.current_fn = d;  // Set current function context

        push_scope(&s);
        // Declare function parameters in scope
        for (int j = 0; j < d->params.count; j++) {
            declare_var(&s, d->params.items[j].name, d->params.items[j].type);
        }
        check_stmts(&d->body, &s);
        pop_scope(&s);

        if (s.scopes) free(s.scopes);
    }
}
