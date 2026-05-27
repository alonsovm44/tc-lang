#include "parser.h"

#include "common.h"
#include "io.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    Token *tokens;
    int pos;
    DeclVec decls;
    char **structs;
    int struct_count;
    int struct_cap;
    char **pinned;
    int pin_count;
    int pin_cap;
} Parser;

static Token *cur(Parser *p) { return &p->tokens[p->pos]; }
static bool at(Parser *p, const char *text) { return strcmp(cur(p)->text, text) == 0; }
static bool match(Parser *p, const char *text) { if (at(p, text)) { p->pos++; return true; } return false; }

static void expect(Parser *p, const char *text) {
    if (!match(p, text)) tc_error("E001", cur(p)->line, cur(p)->col, (int)strlen(cur(p)->text), "expected '%s', got '%s'", text, cur(p)->text);
}

static char *expect_ident(Parser *p) {
    Token *t = cur(p);
    if (t->kind != TOK_IDENT && t->kind != TOK_KEYWORD) tc_error("E002", t->line, t->col, (int)strlen(t->text), "expected identifier, got '%s'", t->text);
    if (t->kind == TOK_KEYWORD && !is_type_name(t->text)) tc_error("E003", t->line, t->col, (int)strlen(t->text), "'%s' is a keyword, not a valid identifier", t->text);
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
        if (at(p, "(") && cur(p)->line == t->line && match(p, "(")) {
            Expr *e = new_expr(EX_CALL);
            e->text = t->text;
            bool is_builtin = !strcmp(t->text, "cast") || !strcmp(t->text, "alloc") || !strcmp(t->text, "sizeof");
            int argn = 0;
            while (!match(p, ")")) {
                bool expect_type = false;
                if (is_builtin) {
                    if (!strcmp(t->text, "cast") && argn == 1) expect_type = true;
                    if (!strcmp(t->text, "alloc") && argn == 0) expect_type = true;
                    if (!strcmp(t->text, "sizeof") && argn == 0) expect_type = true;
                }
                if (expect_type || (is_builtin && (is_type_name(cur(p)->text) || at(p, "->") || at(p, "=>")))) {
                    Expr *arg = new_expr(EX_TYPE);
                    arg->type = parse_type(p);
                    expr_push(&e->args, arg);
                } else {
                    expr_push(&e->args, parse_expr(p));
                }
                match(p, ",");
                argn++;
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
    tc_error("E004", t->line, t->col, (int)strlen(t->text), "expected expression, got '%s'", t->text);
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
        if (match(p, "++")) {
            Expr *inc = new_expr(EX_UNARY);
            inc->text = "p++";
            inc->left = e;
            e = inc;
            continue;
        }
        if (match(p, "--")) {
            Expr *dec = new_expr(EX_UNARY);
            dec->text = "p--";
            dec->left = e;
            e = dec;
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
    if (match(p, "match")) {
        Stmt *s = new_stmt(ST_MATCH);
        expect(p, "("); s->expr = parse_expr(p); expect(p, ")");
        expect(p, "{");
        while (!match(p, "}")) {
            Expr *pattern = NULL;
            if (match(p, "_")) {
                pattern = NULL;  // wildcard
            } else {
                pattern = parse_unary(p);  // parse only a single value, not a full expression
            }
            expect(p, "=");
            StmtVec arm_body = parse_block(p);
            match_arm_push(&s->arms, pattern, arm_body);
        }
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
        if (p->pin_count >= p->pin_cap) {
            p->pin_cap = p->pin_cap ? p->pin_cap * 2 : 8;
            p->pinned = xrealloc(p->pinned, (size_t)p->pin_cap * sizeof(char *));
        }
        p->pinned[p->pin_count++] = s->name;
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
    if (s->expr->kind == EX_BINARY && is_assignment(s->expr->text) && s->expr->left) {
        const char *target = NULL;
        if (s->expr->left->kind == EX_NAME) target = s->expr->left->text;
        if (target) {
            for (int pi = 0; pi < p->pin_count; pi++) {
                if (!strcmp(p->pinned[pi], target)) {
                    Token *t = &p->tokens[mark];
                    tc_error("E000", t->line, t->col, (int)strlen(target),
                        "cannot assign to pinned variable '%s'", target);
                }
            }
        }
    }
    if (s->expr->kind == EX_NAME) {
        Token *t = &p->tokens[mark];
        tc_error("E005", t->line, t->col, (int)strlen(t->text),
            "bare identifier '%s' is not a statement", t->text);
    }
    if (s->expr->kind == EX_LITERAL) {
        Token *t = &p->tokens[mark];
        tc_error("E006", t->line, t->col, (int)strlen(t->text),
            "bare literal '%s' is not a statement", t->text);
    }
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
        param_push(&d->params, pt, pn, false, false);
        if (!match(p, ",")) break;
    }
    p->pin_count = 0;
    d->body = parse_block(p);
    return d;
}

static Decl *parse_struct(Parser *p, bool public, bool packed) {
    Decl *d = new_decl(DC_STRUCT);
    d->public = public;
    d->packed = packed;
    d->name = expect_ident(p);
    add_struct(p, d->name);
    expect(p, "{");
    while (!match(p, "}")) {
        bool is_union_field = false;
        // Check for & prefix (union field in strun)
        if (match(p, "&")) {
            is_union_field = true;
        }
        Type *ft = parse_type(p);
        char *fn = NULL;
        bool is_anonymous = false;
        // Check if this is an anonymous field (no name)
        if (at(p, ";") || at(p, ",")) {
            fn = xstrdup("");  // Empty string for anonymous field
            is_anonymous = true;
        } else {
            fn = expect_ident(p);
        }
        param_push(&d->params, ft, fn, is_union_field, is_anonymous);
        match(p, ",");
    }
    return d;
}

static void resolve_path(const char *base, const char *rel, char *out, size_t out_size) {
    // Find last slash in base path
    const char *last_slash = NULL;
    for (const char *p = base; *p; p++) {
        if (*p == '/' || *p == '\\') last_slash = p;
    }
    if (last_slash) {
        size_t dir_len = (size_t)(last_slash - base + 1);
        if (dir_len >= out_size) dir_len = out_size - 1;
        memcpy(out, base, dir_len);
        out[dir_len] = '\0';
        size_t rem = out_size - dir_len;
        size_t rlen = strlen(rel);
        if (rlen >= rem) rlen = rem - 1;
        memcpy(out + dir_len, rel, rlen);
        out[dir_len + rlen] = '\0';
    } else {
        size_t rlen = strlen(rel);
        if (rlen >= out_size) rlen = out_size - 1;
        memcpy(out, rel, rlen);
        out[rlen] = '\0';
    }
}

DeclVec parse_program(Token *tokens) {
    Parser p = {0};
    p.tokens = tokens;
    while (cur(&p)->kind != TOK_EOF) {
        if (at(&p, "@") && (strcmp((p.tokens + p.pos + 1)->text, "strun") == 0)) {
            p.pos++;  // skip '@'
            match(&p, "strun");
            bool pub = false;
            decl_push(&p.decls, parse_struct(&p, pub, true));
            continue;
        }
        if (at(&p, "@")) {
            Token *at_tok = cur(&p);
            p.pos++;
            if (!match(&p, "use")) tc_error("E007", at_tok->line, at_tok->col, 1, "expected 'use' or 'strun' after '@'");
            if (cur(&p)->kind != TOK_STRING) tc_error("E008", cur(&p)->line, cur(&p)->col, (int)strlen(cur(&p)->text), "'@use' expects a string path, got '%s'", cur(&p)->text);
            char *raw = cur(&p)->text;
            p.pos++;
            char *rel_path = xstrndup(raw + 1, strlen(raw) - 2);
            // Resolve relative to the current source file
            extern const char *g_current_input;
            char resolved[1024];
            if (g_current_input) {
                resolve_path(g_current_input, rel_path, resolved, sizeof(resolved));
            } else {
                snprintf(resolved, sizeof(resolved), "%s", rel_path);
            }
            char *inc_src = try_read_file(resolved);
            if (!inc_src) tc_error("E009", at_tok->line, at_tok->col, 4, "cannot open file '%s'", resolved);
            TokenVec inc_tokens = lex_source(inc_src);
            DeclVec inc_decls = parse_program(inc_tokens.items);
            for (int j = 0; j < inc_decls.count; j++) {
                decl_push(&p.decls, inc_decls.items[j]);
            }
            continue;
        }
        if (match(&p, "use")) {
            Decl *d = new_decl(DC_USE);
            if (cur(&p)->kind != TOK_STRING) tc_error("E010", cur(&p)->line, cur(&p)->col, (int)strlen(cur(&p)->text), "'use' expects a string path, got '%s'", cur(&p)->text);
            d->path = cur(&p)->text;
            p.pos++;
            decl_push(&p.decls, d);
            continue;
        }
        bool public = match(&p, "pub");
        if (match(&p, "extern")) {
            if (cur(&p)->kind != TOK_STRING || strcmp(cur(&p)->text, "\"C\"")) tc_error("E011", cur(&p)->line, cur(&p)->col, (int)strlen(cur(&p)->text), "extern expects \"C\", got '%s'", cur(&p)->text);
            p.pos++;
            expect(&p, "{");
            while (!match(&p, "}")) {
                Type *ret = parse_type(&p);
                expect(&p, "fn");
                decl_push(&p.decls, parse_fn(&p, DC_EXTERN_FN, false, ret));
            }
            continue;
        }
        if (match(&p, "struct") || match(&p, "strun")) {
            decl_push(&p.decls, parse_struct(&p, public, false));
            continue;
        }
        // Check for new fn syntax first
        if (match(&p, "fn")) {
            // New syntax: fn <type> <name>: <type> arg1, <type> arg2, ...
            Type *ret_type = parse_type(&p);
            Decl *d = new_decl(DC_FN);
            d->public = public;
            d->type = ret_type;
            d->name = expect_ident(&p);
            expect(&p, ":");
            
            // Parse parameters
            while (!at(&p, "{")) {
                if (match(&p, "...")) { d->varargs = true; break; }
                Type *pt = parse_type(&p);
                char *pn = expect_ident(&p);
                param_push(&d->params, pt, pn, false, false);
                if (!match(&p, ",")) break;
            }
            
            p.pin_count = 0;
            d->body = parse_block(&p);
            decl_push(&p.decls, d);
        } else {
            // Old syntax: type fn name: ... or variable declaration
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
    }
    return p.decls;
}
