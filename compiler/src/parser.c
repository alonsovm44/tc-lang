#include "parser.h"

#include "common.h"
#include "io.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct {
    Token *tokens;
    int pos;
    DeclVec decls;
    char **structs;
    int struct_count;
    int struct_cap;
    char **enums;
    int enum_count;
    int enum_cap;
    char **pinned;
    int pin_count;
    int pin_cap;
    MacroVec macros;
    const char *source_file; // Source file being parsed
} Parser;

static Token *cur(Parser *p) { return &p->tokens[p->pos]; }
static bool at(Parser *p, const char *text) { return strcmp(cur(p)->text, text) == 0; }
static bool match(Parser *p, const char *text) { if (at(p, text)) { p->pos++; return true; } return false; }

static void expect(Parser *p, const char *text) {
    if (!match(p, text)) tc_error("E001", cur(p)->line, cur(p)->col, (int)strlen(cur(p)->text), "expected '%s', got '%s'", text, cur(p)->text);
}

static void macro_push(Parser *p, Macro *m) {
    if (p->macros.count == p->macros.cap) {
        p->macros.cap = p->macros.cap ? p->macros.cap * 2 : 16;
        p->macros.items = xrealloc(p->macros.items, sizeof(Macro*) * (size_t)p->macros.cap);
    }
    p->macros.items[p->macros.count++] = m;
}

static Macro *find_macro(Parser *p, const char *name) {
    for (int i = p->macros.count - 1; i >= 0; i--) {
        if (strcmp(p->macros.items[i]->name, name) == 0) {
            return p->macros.items[i];
        }
    }
    return NULL;
}

static void parse_macro(Parser *p) {
    Token *macro_token = cur(p);
    char *macro_text = macro_token->text;
    
    // Parse macro text: "# name { replacement }"
    char *start = macro_text + 1; // Skip #
    
    // Skip whitespace after #
    while (*start && isspace((unsigned char)*start)) start++;
    
    // Extract macro name
    char *name_start = start;
    while (*start && !isspace((unsigned char)*start) && *start != '{') start++;
    char *name_end = start;
    
    // Skip whitespace before {
    while (*start && isspace((unsigned char)*start)) start++;
    
    if (*start != '{') {
        tc_error("E001", macro_token->line, macro_token->col, 1, "expected '{' in macro definition");
    }
    start++; // Skip {
    
    // Find matching closing brace
    char *replacement_start = start;
    char *replacement_end = replacement_start;
    int brace_count = 1;
    
    while (*replacement_end && brace_count > 0) {
        if (*replacement_end == '{') brace_count++;
        else if (*replacement_end == '}') brace_count--;
        replacement_end++;
    }
    
    if (brace_count > 0) {
        tc_error("E012", macro_token->line, macro_token->col, 1, "unterminated macro definition");
    }
    
    // Create macro
    Macro *m = xmalloc(sizeof(Macro));
    m->name = xstrndup(name_start, (size_t)(name_end - name_start));
    m->replacement = xstrndup(replacement_start, (size_t)(replacement_end - replacement_start - 1));
    m->params = NULL;
    m->param_count = 0;
    m->is_parametric = false;
    
    macro_push(p, m);
    p->pos++; // Consume the macro token
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

static bool is_enum(Parser *p, const char *name) {
    for (int i = 0; i < p->enum_count; i++) if (strcmp(p->enums[i], name) == 0) return true;
    return false;
}

static void add_enum(Parser *p, char *name) {
    if (p->enum_count == p->enum_cap) {
        p->enum_cap = p->enum_cap ? p->enum_cap * 2 : 8;
        p->enums = xrealloc(p->enums, sizeof(char *) * (size_t)p->enum_cap);
    }
    p->enums[p->enum_count++] = name;
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
    } else if (match(p, "queue")) {
        // Queue type: queue T, queue<T>, or queue T(N)
        t = new_type(TY_QUEUE);
        if (match(p, "<")) {
            // Generic syntax: queue<T>
            t->inner = parse_type(p);
            expect(p, ">");
        } else {
            // Space syntax: queue T
            t->inner = parse_type(p);
        }
        // Check for buffered queue syntax: queue T(N) or queue<T>(N)
        if (match(p, "(")) {
            t->size = parse_expr(p);
            expect(p, ")");
        }
    } else if (match(p, "stack")) {
        // Stack type: stack T or stack<T>
        t = new_type(TY_STACK);
        if (match(p, "<")) {
            // Generic syntax: stack<T>
            t->inner = parse_type(p);
            expect(p, ">");
        } else {
            // Space syntax: stack T
            t->inner = parse_type(p);
        }
    } else if (match(p, "fn")) {
        // Function pointer type: fn(type, type, ...)rettype
        t = new_type(TY_FNPTR);
        expect(p, "(");
        while (!at(p, ")")) {
            Type *pt = parse_type(p);
            param_push(&t->params, pt, xstrdup(""), false, false);
            if (!match(p, ",")) break;
        }
        expect(p, ")");
        t->inner = parse_type(p);  // return type
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
        // Special handling for sizeof
        if (!strcmp(t->text, "sizeof") && at(p, "(") && cur(p)->line == t->line) {
            match(p, "(");  // consume opening parenthesis
            Expr *e = new_expr(EX_SIZEOF);
            e->line = t->line; e->col = t->col;
            // Store the type being measured in a temporary expression
            Expr *type_expr = new_expr(EX_TYPE);
            type_expr->type = parse_type(p);  // Parse the type
            e->left = type_expr;  // Store type expression in left field
            expect(p, ")");  // consume closing parenthesis
            return e;
        }
        if (at(p, "(") && cur(p)->line == t->line && match(p, "(")) {
            Expr *e = new_expr(EX_CALL);
            e->text = t->text;
            e->line = t->line; e->col = t->col;
            bool is_builtin = !strcmp(t->text, "cast") || !strcmp(t->text, "alloc");
            int argn = 0;
            while (!match(p, ")")) {
                bool expect_type = false;
                if (is_builtin) {
                    if (!strcmp(t->text, "cast") && argn == 1) expect_type = true;
                    if (!strcmp(t->text, "alloc") && argn == 0) expect_type = true;
                }
                if (match(p, "...")) {
                    Expr *arg = new_expr(EX_VARARGS);
                    expr_push(&e->args, arg);
                } else if (expect_type || (is_builtin && (is_type_name(cur(p)->text) || at(p, "->") || at(p, "=>")))) {
                    Expr *arg = new_expr(EX_TYPE);
                    arg->type = parse_type(p);
                    expr_push(&e->args, arg);
                } else {
                    Expr *arg = parse_expr(p);
                    // Check if this argument was prefixed with & for pass-by-reference
                    if (arg->kind == EX_UNARY && strcmp(arg->text, "&") == 0) {
                        // Mark it as pass-by-reference by setting a special flag
                        // We'll use the text field to indicate this
                        arg->text = "&ref";
                    }
                    expr_push(&e->args, arg);
                }
                match(p, ",");
                argn++;
            }
            return e;
        }
        Expr *e = new_expr(EX_NAME);
        e->text = t->text;
        e->line = t->line; e->col = t->col;
        return e;
    }
    if (match(p, "(")) {
        // Check if this is a cast: (type)expr
        // A cast starts with a type followed by ) and then an expression
        int save_pos = p->pos;
        Token *open_paren = &p->tokens[save_pos - 1];  // the ( token
        
        // Check if what follows is a type
        if (at(p, "->") || at(p, "=>") || is_type_name(cur(p)->text) || 
            at(p, "queue") || at(p, "stack") || at(p, "fn") ||
            is_struct(p, cur(p)->text) || is_enum(p, cur(p)->text)) {
            // Try to parse a type
            Type *t = parse_type(p);
            if (at(p, ")")) {
                // It's a type followed by ), so it's a cast
                match(p, ")");  // consume the )
                
                Expr *e = new_expr(EX_CAST);
                e->cast_type = t;
                e->left = parse_expr(p);  // parse the expression to cast
                e->line = open_paren->line; e->col = open_paren->col;
                return e;
            } else {
                // Not a cast, restore position
                p->pos = save_pos;
            }
        }
        
        // Not a cast, just a parenthesized expression
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
            if (match(p, ">")) {
                // .> for pointer field access or method call
                char *field_name = expect_ident(p);
                if (match(p, "(")) {
                    // Check if this is a queue/stack method call on a pointer
                    bool is_queue_stack_method = false;
                    if (e->type && e->type->kind == TY_RAWPTR) {
                        Type *inner = e->type->inner;
                        if (inner && (inner->kind == TY_QUEUE || inner->kind == TY_STACK)) {
                            is_queue_stack_method = true;
                        }
                    }

                    if (is_queue_stack_method) {
                        Expr *method_call = new_expr(EX_QUEUE_METHOD);
                        method_call->left = e;
                        method_call->text = field_name;

                        // Parse arguments
                        while (!match(p, ")")) {
                            expr_push(&method_call->args, parse_expr(p));
                            match(p, ",");
                        }
                        e = method_call;
                    } else {
                        // Regular method call on pointer
                        Expr *method_call = new_expr(EX_METHOD_CALL);
                        method_call->left = e;
                        method_call->text = field_name;

                        while (!match(p, ")")) {
                            expr_push(&method_call->args, parse_expr(p));
                            match(p, ",");
                        }
                        e = method_call;
                    }
                } else {
                    // Regular pointer field access
                    Expr *field = new_expr(EX_PTR_FIELD);
                    field->left = e;
                    field->text = field_name;
                    e = field;
                }
            } else {
                // Check if this is a method call: expr.method(...)
                char *field_name = expect_ident(p);
                if (match(p, "(")) {
                    // Check if this is a queue/stack method call
                    bool is_queue_stack_method = false;
                    if (e->type && (e->type->kind == TY_QUEUE || e->type->kind == TY_STACK)) {
                        is_queue_stack_method = true;
                    }
                    
                    if (is_queue_stack_method) {
                        Expr *method_call = new_expr(EX_QUEUE_METHOD);
                        method_call->left = e;
                        method_call->text = field_name;
                        
                        // Parse arguments
                        while (!match(p, ")")) {
                            expr_push(&method_call->args, parse_expr(p));
                            match(p, ",");
                        }
                        e = method_call;
                    } else {
                        Expr *method_call = new_expr(EX_METHOD_CALL);
                        method_call->left = e;
                        method_call->text = field_name;
                        
                        // Store struct name in type->name for emitter to use
                        if (e->type && e->type->name) {
                            method_call->type = new_type(TY_NAME);
                            method_call->type->name = xstrdup(e->type->name);
                        }
                        
                        // Parse arguments
                        while (!match(p, ")")) {
                            expr_push(&method_call->args, parse_expr(p));
                            match(p, ",");
                        }
                        e = method_call;
                    }
                } else {
                    // Regular field access
                    Expr *field = new_expr(EX_FIELD);
                    field->left = e;
                    field->text = field_name;
                    e = field;
                }
            }
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
    if (at(p, "!") || at(p, "@") || at(p, "-") || at(p, "++") || at(p, "--") || at(p, "->") || at(p, "&")) {
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
    if (match(p, "select")) {
        Stmt *s = new_stmt(ST_SELECT);
        expect(p, "{");
        while (!match(p, "}")) {
            // Parse select case: operation => { body }
            parse_expr(p);
            expect(p, "=>");
            StmtVec case_body = {0};
            if (match(p, "{")) {
                while (!match(p, "}")) {
                    stmt_push(&case_body, parse_stmt(p));
                }
            } else {
                // Single statement case
                stmt_push(&case_body, parse_stmt(p));
            }
            // Add case to select statement
            // We'll store cases in the body vector for now
            stmt_push(&s->body, new_stmt(ST_EXPR));  // Placeholder
            match(p, ",");
        }
        return s;
    }
    if (match(p, "if")) {
        Stmt *s = new_stmt(ST_IF);
        expect(p, "("); s->expr = parse_expr(p); expect(p, ")");
        s->body = parse_block(p);
        // Parse else if(...){} chains and final else{}
        while (match(p, "else")) {
            if (match(p, "if")) {
                // else if
                expect(p, "("); Expr *cond = parse_expr(p); expect(p, ")");
                StmtVec arm_body = parse_block(p);
                elseif_push(&s->elseifs, cond, arm_body);
            } else {
                // just else
                s->else_body = parse_block(p);
                break;
            }
        }
        return s;
    }
    if (match(p, "loop")) {
        Stmt *s = new_stmt(ST_LOOP);
        if (match(p, "if")) { expect(p, "("); s->expr = parse_expr(p); expect(p, ")"); }
        s->body = parse_block(p);
        return s;
    }
    if (match(p, "while")) {
        Stmt *s = new_stmt(ST_LOOP);
        expect(p, "("); s->expr = parse_expr(p); expect(p, ")");
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
        // Support both braced and braceless defer
        if (at(p, "{")) {
            s->body = parse_block(p);
        } else {
            // Braceless defer: parse single statement
            Stmt *single_stmt = parse_stmt(p);
            stmt_push(&s->body, single_stmt);
        }
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
    if (match(p, "throw")) {
        Stmt *s = new_stmt(ST_THROW);
        s->expr = parse_expr(p);
        return s;
    }
    if (match(p, "try")) {
        Stmt *s = new_stmt(ST_TRY);
        s->body = parse_block(p);
        expect(p, "catch");
        expect(p, "{");
        while (!match(p, "}")) {
            // Parse catch arm: ErrorName(args) ret X, or _ { body }
            char *error_name = NULL;
            ExprVec args = {0};
            Expr *ret_expr = NULL;
            StmtVec arm_body = {0};
            
            if (match(p, "_")) {
                // Wildcard catch
                error_name = NULL;
            } else {
                // Specific error catch
                error_name = expect_ident(p);
                if (match(p, "(")) {
                    // Parse arguments to match
                    if (!at(p, ")")) {
                        do {
                            expr_push(&args, parse_expr(p));
                        } while (match(p, ","));
                    }
                    expect(p, ")");
                }
            }
            
            // Check for return expression
            if (match(p, "ret")) {
                ret_expr = parse_expr(p);
            }
            
            // Parse body (optional for simple catch arms)
            if (at(p, "{")) {
                arm_body = parse_block(p);
            }
            
            catch_arm_push(&s->catch_arms, error_name, args, arm_body, ret_expr);
            match(p, ",");
        }
        return s;
    }
    if (match(p, "{")) {
        Stmt *s = new_stmt(ST_BLOCK);
        while (!match(p, "}")) stmt_push(&s->body, parse_stmt(p));
        return s;
    }
    // Handle inline C code
    if (cur(p)->kind == TOK_INLINE_C) {
        Stmt *s = new_stmt(ST_INLINE_C);
        s->text = xstrdup(cur(p)->text);
        p->pos++;
        return s;
    }
    int mark = p->pos;
    bool is_fn_type = at(p, "fn");
    bool type_start = is_type_name(cur(p)->text) || at(p, "->") || at(p, "=>") || is_fn_type || is_struct(p, cur(p)->text) || is_enum(p, cur(p)->text) || at(p, "queue") || at(p, "stack");
    if (type_start) {
        Type *type = parse_type(p);
        if (cur(p)->kind == TOK_IDENT) {
            Stmt *s = new_stmt(ST_VAR);
            s->type = type;
            s->name = expect_ident(p);
            if (match(p, "=")) s->expr = parse_initializer(p);
            match(p, ";");  // Semicolon is optional after variable declaration
            return s;
        }
        if (!is_fn_type) p->pos = mark;
    }
    if (is_fn_type) { Token *t = &p->tokens[mark]; tc_error("E002", t->line, t->col, 2, "expected variable name after function pointer type"); }
    p->pos = mark;
    Stmt *s = new_stmt(ST_EXPR);
    s->expr = parse_expr(p);
    match(p, ";");  // Semicolon is optional after expressions
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

static Decl *parse_fn(Parser *p, DeclKind kind, bool is_my, Type *ret) {
    Decl *d = new_decl(kind);
    d->is_my = is_my;
    d->source_file = xstrdup(p->source_file);
    if (ret) {
        // Old syntax: return_type fn name: params
        d->type = ret;
        d->name = expect_ident(p);
        expect(p, ":");
    } else {
        // New syntax: fn return_type name: params
        // Check if next token is a type (for new syntax) or name (for old syntax without return type)
        if (is_type_name(cur(p)->text)) {
            d->type = parse_type(p);
            d->name = expect_ident(p);
            expect(p, ":");
        } else {
            d->name = expect_ident(p);
            expect(p, ":");
            d->type = parse_type(p);
        }
    }
    while (!at(p, "{") && !at(p, ";") && !at(p, "}")) {
        if (match(p, "...")) { d->varargs = true; break; }
        Type *pt = parse_type(p);
        char *pn = expect_ident(p);
        param_push(&d->params, pt, pn, false, false);
        if (!match(p, ",")) break;
    }
    p->pin_count = 0;
    if (kind == DC_EXTERN_FN) {
        match(p, ";");  // Optional semicolon for extern declarations
        d->body.count = 0;  // No body for extern functions
    } else {
        d->body = parse_block(p);
    }
    return d;
}

static Decl *parse_struct(Parser *p, bool is_my, bool packed) {
    Decl *d = new_decl(DC_STRUCT);
    d->is_my = is_my;
    d->source_file = xstrdup(p->source_file);
    d->packed = packed;
    d->name = expect_ident(p);
    add_struct(p, d->name);
    expect(p, "{");
    while (!match(p, "}")) {
        // Check for method definition
        if (match(p, "fn")) {
            // Parse return type
            Type *ret = parse_type(p);
            // Parse method name
            char *method_name = expect_ident(p);
            expect(p, ":");
            // Create method declaration
            Decl *method = new_decl(DC_FN);
            method->name = method_name;
            method->type = ret;
            // Parse parameters (implicit self will be added later)
            while (!at(p, "{")) {
                if (match(p, "...")) { method->varargs = true; break; }
                Type *pt = parse_type(p);
                char *pn = expect_ident(p);
                param_push(&method->params, pt, pn, false, false);
                if (!match(p, ",")) break;
            }
            p->pin_count = 0;
            method->body = parse_block(p);
            decl_push(&d->methods, method);
        } else {
            // Parse field
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
    }
    return d;
}

static Decl *parse_enum(Parser *p, bool is_my) {
    Decl *d = new_decl(DC_ENUM);
    d->is_my = is_my;
    d->source_file = xstrdup(p->source_file);
    // Check for explicit type (e.g., enum u8 Color)
    if (is_type_name(cur(p)->text)) {
        d->type = parse_type(p);
    } else {
        // Default to i32
        d->type = new_type(TY_NAME);
        d->type->name = xstrdup("i32");
    }
    d->name = expect_ident(p);
    add_enum(p, d->name);
    expect(p, "{");
    while (!match(p, "}")) {
        char *member_name = expect_ident(p);
        enum_member_push(&d->enum_members, member_name);
        match(p, ",");
    }
    return d;
}

static Decl *parse_error(Parser *p, bool is_my) {
    Decl *d = new_decl(DC_ERROR);
    d->is_my = is_my;
    d->source_file = xstrdup(p->source_file);
    d->name = expect_ident(p);
    expect(p, ":");
    // Parse parameters (like a function)
    while (!at(p, "{")) {
        Type *pt = parse_type(p);
        char *pn = expect_ident(p);
        param_push(&d->params, pt, pn, false, false);
        if (!match(p, ",")) break;
    }
    // Parse error handler body
    d->body = parse_block(p);
    return d;
}

static void resolve_path(const char *base, const char *rel, char *out, size_t out_size) {
    // If rel starts with the same directory as base's directory, use as-is
    // Or just check if rel contains the base directory name? This is messy.
    
    // Simpler approach: just get the directory of base, then append the filename part of rel
    const char *last_slash_base = NULL;
    for (const char *p = base; *p; p++) {
        if (*p == '/' || *p == '\\') last_slash_base = p;
    }
    
    const char *last_slash_rel = NULL;
    for (const char *p = rel; *p; p++) {
        if (*p == '/' || *p == '\\') last_slash_rel = p;
    }
    
    if (last_slash_base && last_slash_rel) {
        // rel has a path, use it directly (it's already relative to project root)
        strncpy(out, rel, out_size);
    } else if (last_slash_base) {
        // rel is just a filename, combine with base directory
        size_t dir_len = (size_t)(last_slash_base - base + 1);
        if (dir_len >= out_size) dir_len = out_size - 1;
        memcpy(out, base, dir_len);
        out[dir_len] = '\0';
        strncat(out, rel, out_size - dir_len - 1);
    } else {
        strncpy(out, rel, out_size);
    }
}

DeclVec parse_program(Token *tokens, const char *source_file) {
    Parser p = {0};
    p.tokens = tokens;
    p.source_file = source_file;
    while (cur(&p)->kind != TOK_EOF) {
        if (at(&p, "@") && (strcmp((p.tokens + p.pos + 1)->text, "strun") == 0)) {
            p.pos++;  // skip '@'
            match(&p, "strun");
            decl_push(&p.decls, parse_struct(&p, false, true));
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
            DeclVec inc_decls = parse_program(inc_tokens.items, resolved);
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
        // Check for global inline C: TOK_INLINE_C token
        if (cur(&p)->kind == TOK_INLINE_C) {
            Token *c_tok = cur(&p);
            p.pos++;  // consume the TOK_INLINE_C token
            Decl *d = new_decl(DC_INLINE_C);
            d->text = c_tok->text;
            decl_push(&p.decls, d);
            continue;
        }
        // Check for macro definitions: TOK_MACRO token
        if (cur(&p)->kind == TOK_MACRO) {
            parse_macro(&p);
            continue;
        }
        if (match(&p, "extern")) {
            if (cur(&p)->kind != TOK_STRING || strcmp(cur(&p)->text, "\"C\"")) tc_error("E011", cur(&p)->line, cur(&p)->col, (int)strlen(cur(&p)->text), "extern expects \"C\", got '%s'", cur(&p)->text);
            p.pos++;
            expect(&p, "{");
            while (!at(&p, "}")) {
                // Check for old syntax: return_type fn name: params
                // or new syntax: fn return_type name: params
                Type *ret = NULL;
                if (match(&p, "fn")) {
                    // New syntax: fn return_type name: params
                    ret = NULL;  // parse_fn will parse return type after name
                } else {
                    // Old syntax: return_type fn name: params
                    ret = parse_type(&p);
                    expect(&p, "fn");
                }
                decl_push(&p.decls, parse_fn(&p, DC_EXTERN_FN, false, ret));
            }
            match(&p, "}");
            continue;
        }
        if (match(&p, "struct") || match(&p, "strun")) {
            decl_push(&p.decls, parse_struct(&p, false, false));
            continue;
        }
        if (match(&p, "enum")) {
            decl_push(&p.decls, parse_enum(&p, false));
            continue;
        }
        if (match(&p, "error")) {
            decl_push(&p.decls, parse_error(&p, false));
            continue;
        }
        // Check for async fn syntax first
        bool is_async = false;
        if (match(&p, "async")) {
            is_async = true;
        }
        
        if (match(&p, "fn")) {
            // New syntax: [async] fn <type> <name>: <type> arg1, <type> arg2, ...
            Type *ret_type = parse_type(&p);
            Decl *d = new_decl(DC_FN);
            d->is_async = is_async;
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
            // Old syntax: [async] type fn name: ... or variable declaration
            bool old_async = false;
            if (is_async && !match(&p, "fn")) {
                // If we saw async but it's not followed by fn, it's a variable declaration
                // Reset is_async for the next iteration
                is_async = false;
            }
            
            Type *type = parse_type(&p);
            if (match(&p, "fn")) {
                Decl *fn_decl = parse_fn(&p, DC_FN, false, type);
                fn_decl->is_async = is_async;
                decl_push(&p.decls, fn_decl);
            } else {
                Decl *d = new_decl(DC_VAR);
                d->type = type;
                d->name = expect_ident(&p);
                if (match(&p, "=")) d->init = parse_initializer(&p);
                decl_push(&p.decls, d);
            }
        }
    }
    return p.decls;
}
