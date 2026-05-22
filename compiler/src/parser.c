#include "parser.h"

#include "common.h"

#include <string.h>

typedef struct {
    Token *tokens;
    int pos;
    DeclVec decls;
    char **structs;
    int struct_count;
    int struct_cap;
} Parser;

static Token *cur(Parser *p) { return &p->tokens[p->pos]; }
static bool at(Parser *p, const char *text) { return strcmp(cur(p)->text, text) == 0; }
static bool match(Parser *p, const char *text) { if (at(p, text)) { p->pos++; return true; } return false; }

static void expect(Parser *p, const char *text) {
    if (!match(p, text)) die("expected '%s' at %d:%d, got '%s'", text, cur(p)->line, cur(p)->col, cur(p)->text);
}

static char *expect_ident(Parser *p) {
    Token *t = cur(p);
    if (t->kind != TOK_IDENT && t->kind != TOK_KEYWORD) die("expected identifier at %d:%d", t->line, t->col);
    if (t->kind == TOK_KEYWORD && !is_type_name(t->text)) die("expected identifier at %d:%d", t->line, t->col);
    p->pos++;
    return t->text;
}

static bool is_struct(Parser *p, const char *name) {
    for (int i = 0; i < p->struct_count; i++) if (strcmp(p->structs[i], name) == 0) return true;
    return false;
}

static void add_struct(Parser *p, char *name) {
    if (p->struct_count == p->struct_cap) {
        p->struct_cap = p->struct_cap ? p->struct_cap * 2 : 8;
        p->structs = xrealloc(p->structs, sizeof(char *) * (size_t)p->struct_cap);
    }
    p->structs[p->struct_count++] = name;
}

static Expr *parse_expr(Parser *p);

static Type *parse_type(Parser *p) {
    Type *t;
    if (match(p, "->")) {
        t = new_type(TY_RAWPTR);
        t->inner = parse_type(p);
    } else if (match(p, "=>")) {
        t = new_type(TY_FATPTR);
        t->inner = parse_type(p);
    } else {
        char *name = expect_ident(p);
        t = new_type(TY_NAME);
        t->name = name;
    }
    while (match(p, "[")) {
        Type *arr = new_type(TY_ARRAY);
        arr->inner = t;
        arr->size = parse_expr(p);
        expect(p, "]");
        t = arr;
    }
    return t;
}

static int precedence(const char *op) {
    if (!strcmp(op, "=") || !strcmp(op, "+=") || !strcmp(op, "-=") || !strcmp(op, "*=") || !strcmp(op, "/=") || !strcmp(op, "%=")) return 1;
    if (!strcmp(op, "||")) return 2;
    if (!strcmp(op, "&&")) return 3;
    if (!strcmp(op, "==") || !strcmp(op, "<>")) return 4;
    if (!strcmp(op, "<") || !strcmp(op, ">") || !strcmp(op, "<=") || !strcmp(op, ">=")) return 5;
    if (!strcmp(op, "<<") || !strcmp(op, ">>")) return 6;
    if (!strcmp(op, "+") || !strcmp(op, "-")) return 7;
    if (!strcmp(op, "*") || !strcmp(op, "/") || !strcmp(op, "%")) return 8;
    if (!strcmp(op, "&") || !strcmp(op, "|") || !strcmp(op, "^")) return 9;
    return 0;
}

static bool is_assignment(const char *op) {
    return !strcmp(op, "=") || !strcmp(op, "+=") || !strcmp(op, "-=") || !strcmp(op, "*=") || !strcmp(op, "/=") || !strcmp(op, "%=");
}

static Expr *parse_primary(Parser *p) {
    Token *t = cur(p);
    if (t->kind == TOK_NUMBER || t->kind == TOK_STRING) {
        p->pos++;
        Expr *e = new_expr(EX_LITERAL);
        e->text = t->text;
        return e;
    }
    if (t->kind == TOK_IDENT || t->kind == TOK_KEYWORD) {
        p->pos++;
        if (match(p, "(")) {
            Expr *e = new_expr(EX_CALL);
            e->text = t->text;
            while (!match(p, ")")) {
                if (is_type_name(cur(p)->text)) {
                    Expr *arg = new_expr(EX_TYPE);
                    arg->type = parse_type(p);
                    expr_push(&e->args, arg);
                } else {
                    expr_push(&e->args, parse_expr(p));
                }
                match(p, ",");
            }
            return e;
        }
        Expr *e = new_expr(EX_NAME);
        e->text = t->text;
        return e;
    }
    if (match(p, "(")) {
        Expr *e = parse_expr(p);
        expect(p, ")");
        return e;
    }
    die("expected expression at %d:%d, got '%s'", t->line, t->col, t->text);
    return NULL;
}

static Expr *parse_postfix(Parser *p) {
    Expr *e = parse_primary(p);
    for (;;) {
        if (match(p, "[")) {
            Expr *start = parse_expr(p);
            if (match(p, ":")) {
                Expr *end = parse_expr(p);
                expect(p, "]");
                Expr *slice = new_expr(EX_SLICE);
                slice->left = e;
                slice->right = start;
                slice->third = end;
                e = slice;
            } else {
                expect(p, "]");
                Expr *idx = new_expr(EX_INDEX);
                idx->left = e;
                idx->right = start;
                e = idx;
            }
            continue;
        }
        if (match(p, ".")) {
            Expr *field = new_expr(EX_FIELD);
            field->left = e;
            field->text = expect_ident(p);
            e = field;
            continue;
        }
        break;
    }
    return e;
}

static Expr *parse_unary(Parser *p) {
    if (at(p, "!") || at(p, "@") || at(p, "-") || at(p, "++") || at(p, "--") || at(p, "->")) {
        Expr *e = new_expr(EX_UNARY);
        e->text = cur(p)->text;
        p->pos++;
        e->left = parse_unary(p);
        return e;
    }
    return parse_postfix(p);
}

static Expr *parse_binary(Parser *p, int min_prec) {
    Expr *left = parse_unary(p);
    for (;;) {
        int prec = precedence(cur(p)->text);
        if (prec < min_prec || prec == 0) break;
        char *op = cur(p)->text;
        p->pos++;
        Expr *right = parse_binary(p, prec + (is_assignment(op) ? 0 : 1));
        Expr *bin = new_expr(EX_BINARY);
        bin->text = op;
        bin->left = left;
        bin->right = right;
        left = bin;
    }
    return left;
}

static Expr *parse_expr(Parser *p) {
    return parse_binary(p, 1);
}

static Expr *parse_initializer(Parser *p) {
    if (match(p, "{")) {
        Expr *e = new_expr(EX_INIT_LIST);
        while (!match(p, "}")) {
            expr_push(&e->args, parse_expr(p));
            match(p, ",");
        }
        return e;
    }
    return parse_expr(p);
}

static StmtVec parse_block(Parser *p);

static Stmt *parse_stmt(Parser *p) {
    if (match(p, "if")) {
        Stmt *s = new_stmt(ST_IF);
        expect(p, "("); s->expr = parse_expr(p); expect(p, ")");
        s->body = parse_block(p);
        return s;
    }
    if (match(p, "loop")) {
        Stmt *s = new_stmt(ST_LOOP);
        if (match(p, "if")) { expect(p, "("); s->expr = parse_expr(p); expect(p, ")"); }
        s->body = parse_block(p);
        return s;
    }
    if (match(p, "defer")) {
        Stmt *s = new_stmt(ST_DEFER);
        s->body = parse_block(p);
        return s;
    }
    if (match(p, "ret")) {
        Stmt *s = new_stmt(ST_RET);
        if (!at(p, "}")) s->expr = parse_expr(p);
        return s;
    }
    if (match(p, "break")) return new_stmt(ST_BREAK);
    if (match(p, "pin")) {
        Stmt *s = new_stmt(ST_PIN);
        s->name = expect_ident(p);
        return s;
    }
    if (match(p, "{")) {
        Stmt *s = new_stmt(ST_BLOCK);
        while (!match(p, "}")) stmt_push(&s->body, parse_stmt(p));
        return s;
    }
    int mark = p->pos;
    bool type_start = is_type_name(cur(p)->text) || at(p, "->") || at(p, "=>") || is_struct(p, cur(p)->text);
    if (type_start) {
        Type *type = parse_type(p);
        if (cur(p)->kind == TOK_IDENT || cur(p)->kind == TOK_KEYWORD) {
            Stmt *s = new_stmt(ST_VAR);
            s->type = type;
            s->name = expect_ident(p);
            if (match(p, "=")) s->expr = parse_initializer(p);
            return s;
        }
    }
    p->pos = mark;
    Stmt *s = new_stmt(ST_EXPR);
    s->expr = parse_expr(p);
    return s;
}

static StmtVec parse_block(Parser *p) {
    StmtVec body = {0};
    expect(p, "{");
    while (!match(p, "}")) stmt_push(&body, parse_stmt(p));
    return body;
}

static Decl *parse_fn(Parser *p, DeclKind kind, bool public, Type *ret) {
    Decl *d = new_decl(kind);
    d->public = public;
    d->type = ret;
    d->name = expect_ident(p);
    expect(p, ":");
    while (!at(p, "{")) {
        if (match(p, "...")) { d->varargs = true; break; }
        Type *pt = parse_type(p);
        char *pn = expect_ident(p);
        param_push(&d->params, pt, pn);
        if (!match(p, ",")) break;
    }
    d->body = parse_block(p);
    return d;
}

static Decl *parse_struct(Parser *p, bool public) {
    Decl *d = new_decl(DC_STRUCT);
    d->public = public;
    d->name = expect_ident(p);
    add_struct(p, d->name);
    expect(p, "{");
    while (!match(p, "}")) {
        Type *ft = parse_type(p);
        char *fn = expect_ident(p);
        param_push(&d->params, ft, fn);
        match(p, ",");
    }
    return d;
}

DeclVec parse_program(Token *tokens) {
    Parser p = {0};
    p.tokens = tokens;
    while (cur(&p)->kind != TOK_EOF) {
        if (match(&p, "use")) {
            Decl *d = new_decl(DC_USE);
            if (cur(&p)->kind != TOK_STRING) die("use expects a string literal at %d:%d", cur(&p)->line, cur(&p)->col);
            d->path = cur(&p)->text;
            p.pos++;
            decl_push(&p.decls, d);
            continue;
        }
        bool public = match(&p, "pub");
        if (match(&p, "extern")) {
            if (cur(&p)->kind != TOK_STRING || strcmp(cur(&p)->text, "\"C\"")) die("extern expects \"C\"");
            p.pos++;
            expect(&p, "{");
            while (!match(&p, "}")) {
                Type *ret = parse_type(&p);
                expect(&p, "fn");
                decl_push(&p.decls, parse_fn(&p, DC_EXTERN_FN, false, ret));
            }
            continue;
        }
        if (match(&p, "struct")) {
            decl_push(&p.decls, parse_struct(&p, public));
            continue;
        }
        Type *type = parse_type(&p);
        if (match(&p, "fn")) {
            decl_push(&p.decls, parse_fn(&p, DC_FN, public, type));
        } else {
            Decl *d = new_decl(DC_VAR);
            d->public = public;
            d->type = type;
            d->name = expect_ident(&p);
            if (match(&p, "=")) d->init = parse_initializer(&p);
            decl_push(&p.decls, d);
        }
    }
    return p.decls;
}
