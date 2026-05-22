#include "emitter.h"

#include "common.h"

#include <stdlib.h>
#include <string.h>

static bool struct_exists(DeclVec *program, const char *name) {
    for (int i = 0; i < program->count; i++) if (program->items[i]->kind == DC_STRUCT && strcmp(program->items[i]->name, name) == 0) return true;
    return false;
}

static void emit_expr(Str *out, Expr *e, DeclVec *program);

static void emit_type(Str *out, Type *t, const char *name, DeclVec *program) {
    if (t->kind == TY_RAWPTR) {
        Str inner = {0};
        str_printf(&inner, "*%s", name ? name : "");
        emit_type(out, t->inner, inner.data, program);
        return;
    }
    if (t->kind == TY_FATPTR) {
        Str base = {0};
        emit_type(&base, t->inner, "", program);
        str_printf(out, "struct { %s *ptr; size_t len; } %s", base.data, name ? name : "");
        return;
    }
    if (t->kind == TY_ARRAY) {
        Str arr = {0};
        str_printf(&arr, "%s[", name ? name : "");
        emit_expr(&arr, t->size, program);
        str_add(&arr, "]");
        emit_type(out, t->inner, arr.data, program);
        return;
    }
    const char *base = t->name;
    if (!strcmp(base, "void")) base = "void";
    else if (!strcmp(base, "i2") || !strcmp(base, "i4")) base = "int8_t";
    else if (!strcmp(base, "i8")) base = "char";
    else if (!strcmp(base, "i16")) base = "int16_t";
    else if (!strcmp(base, "i32")) base = "int32_t";
    else if (!strcmp(base, "i64")) base = "int64_t";
    else if (!strcmp(base, "u2") || !strcmp(base, "u4") || !strcmp(base, "u8")) base = "uint8_t";
    else if (!strcmp(base, "u16")) base = "uint16_t";
    else if (!strcmp(base, "u32")) base = "uint32_t";
    else if (!strcmp(base, "u64")) base = "uint64_t";
    else if (!strcmp(base, "f32")) base = "float";
    else if (!strcmp(base, "f64")) base = "double";
    if (struct_exists(program, base)) str_printf(out, "struct %s", base);
    else str_add(out, base);
    if (name && name[0]) str_printf(out, " %s", name);
}

static void emit_expr(Str *out, Expr *e, DeclVec *program) {
    if (!e) return;
    switch (e->kind) {
        case EX_NAME: str_add(out, e->text); break;
        case EX_LITERAL:
            if (!strncmp(e->text, "0b", 2)) str_printf(out, "%ld", strtol(e->text + 2, NULL, 2));
            else str_add(out, e->text);
            break;
        case EX_BINARY:
            str_add(out, "("); emit_expr(out, e->left, program); str_printf(out, " %s ", !strcmp(e->text, "<>") ? "!=" : e->text); emit_expr(out, e->right, program); str_add(out, ")");
            break;
        case EX_UNARY:
            str_printf(out, "(%s", !strcmp(e->text, "->") ? "*" : !strcmp(e->text, "@") ? "&" : e->text); emit_expr(out, e->left, program); str_add(out, ")");
            break;
        case EX_CALL:
            if (!strcmp(e->text, "sizeof") && e->args.count == 1 && e->args.items[0]->kind == EX_TYPE) {
                str_add(out, "sizeof("); emit_type(out, e->args.items[0]->type, "", program); str_add(out, ")");
                break;
            }
            if (!strcmp(e->text, "alloc") && e->args.count == 2 && e->args.items[0]->kind == EX_TYPE) {
                str_add(out, "TC_ALLOC("); emit_type(out, e->args.items[0]->type, "", program); str_add(out, ", "); emit_expr(out, e->args.items[1], program); str_add(out, ")");
                break;
            }
            str_printf(out, "%s(", e->text);
            for (int i = 0; i < e->args.count; i++) { if (i) str_add(out, ", "); emit_expr(out, e->args.items[i], program); }
            str_add(out, ")");
            break;
        case EX_INDEX: emit_expr(out, e->left, program); str_add(out, "["); emit_expr(out, e->right, program); str_add(out, "]"); break;
        case EX_FIELD: emit_expr(out, e->left, program); str_printf(out, ".%s", e->text); break;
        case EX_SLICE: str_add(out, "{ &"); emit_expr(out, e->left, program); str_add(out, "["); emit_expr(out, e->right, program); str_add(out, "], ("); emit_expr(out, e->third, program); str_add(out, " - "); emit_expr(out, e->right, program); str_add(out, ") }"); break;
        case EX_INIT_LIST:
            str_add(out, "{");
            for (int i = 0; i < e->args.count; i++) { if (i) str_add(out, ", "); emit_expr(out, e->args.items[i], program); }
            str_add(out, "}");
            break;
        case EX_TYPE: emit_type(out, e->type, "", program); break;
    }
}

static void emit_indent(Str *out, int indent) {
    for (int i = 0; i < indent; i++) str_add(out, "    ");
}

static void emit_stmt_vec(Str *out, StmtVec *body, DeclVec *program, int indent);

static void emit_defers(Str *out, StmtVec *body, DeclVec *program, int indent) {
    for (int i = body->count - 1; i >= 0; i--) {
        Stmt *s = body->items[i];
        if (s->kind == ST_DEFER) emit_stmt_vec(out, &s->body, program, indent);
    }
}

static void emit_stmt(Str *out, Stmt *s, StmtVec *scope, DeclVec *program, int indent) {
    emit_indent(out, indent);
    switch (s->kind) {
        case ST_VAR: {
            emit_type(out, s->type, s->name, program);
            if (s->expr) { str_add(out, " = "); emit_expr(out, s->expr, program); }
            else str_add(out, " = {0}");
            str_add(out, ";\n");
            break;
        }
        case ST_IF:
            str_add(out, "if ("); emit_expr(out, s->expr, program); str_add(out, ") {\n"); emit_stmt_vec(out, &s->body, program, indent + 1); emit_indent(out, indent); str_add(out, "}\n");
            break;
        case ST_LOOP:
            str_add(out, "while ("); if (s->expr) emit_expr(out, s->expr, program); else str_add(out, "1"); str_add(out, ") {\n"); emit_stmt_vec(out, &s->body, program, indent + 1); emit_indent(out, indent); str_add(out, "}\n");
            break;
        case ST_BLOCK:
            str_add(out, "{\n"); emit_stmt_vec(out, &s->body, program, indent + 1); emit_indent(out, indent); str_add(out, "}\n");
            break;
        case ST_RET:
            str_add(out, "\n"); emit_defers(out, scope, program, indent); emit_indent(out, indent); str_add(out, "return"); if (s->expr) { str_add(out, " "); emit_expr(out, s->expr, program); } str_add(out, ";\n");
            break;
        case ST_BREAK: str_add(out, "break;\n"); break;
        case ST_PIN: break;
        case ST_EXPR: emit_expr(out, s->expr, program); str_add(out, ";\n"); break;
        case ST_DEFER: break;
    }
}

static void emit_stmt_vec(Str *out, StmtVec *body, DeclVec *program, int indent) {
    for (int i = 0; i < body->count; i++) if (body->items[i]->kind != ST_DEFER) emit_stmt(out, body->items[i], body, program, indent);
}

char *emit_program(DeclVec program) {
    Str out = {0};
    str_add(&out, "#include <stdint.h>\n#include <stddef.h>\n#include <stdio.h>\n#include <stdlib.h>\n#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))\n#define lenof(x) (sizeof(x) / sizeof((x)[0]))\n\n");
    for (int i = 0; i < program.count; i++) {
        Decl *d = program.items[i];
        if (d->kind == DC_USE) {
            char *path = xstrndup(d->path + 1, strlen(d->path) - 2);
            size_t plen = strlen(path);
            if (plen > 3 && strcmp(path + plen - 3, ".tc") == 0) {
                path[plen - 2] = 'h';
                path[plen - 1] = '\0';
            }
            str_printf(&out, "#include \"%s\"\n", path);
        }
    }
    for (int i = 0; i < program.count; i++) {
        Decl *d = program.items[i];
        if (d->kind == DC_STRUCT) {
            str_printf(&out, "struct %s {\n", d->name);
            for (int j = 0; j < d->params.count; j++) {
                str_add(&out, "    "); emit_type(&out, d->params.items[j].type, d->params.items[j].name, &program); str_add(&out, ";\n");
            }
            str_add(&out, "};\n\n");
        }
    }
    for (int i = 0; i < program.count; i++) {
        Decl *d = program.items[i];
        if (d->kind == DC_FN) {
            emit_type(&out, d->type, d->name, &program); str_add(&out, "(");
            if (!d->params.count && !d->varargs) str_add(&out, "void");
            for (int j = 0; j < d->params.count; j++) { if (j) str_add(&out, ", "); emit_type(&out, d->params.items[j].type, d->params.items[j].name, &program); }
            if (d->varargs) str_add(&out, d->params.count ? ", ..." : "...");
            str_add(&out, ");\n");
        }
    }
    str_add(&out, "\n");
    for (int i = 0; i < program.count; i++) {
        Decl *d = program.items[i];
        if (d->kind == DC_VAR) {
            emit_type(&out, d->type, d->name, &program);
            if (d->init) {
                str_add(&out, " = ");
                emit_expr(&out, d->init, &program);
            }
            str_add(&out, ";\n");
        }
    }
    for (int i = 0; i < program.count; i++) {
        Decl *d = program.items[i];
        if (d->kind == DC_FN) {
            str_add(&out, "\n"); emit_type(&out, d->type, d->name, &program); str_add(&out, "(");
            if (!d->params.count) str_add(&out, "void");
            for (int j = 0; j < d->params.count; j++) { if (j) str_add(&out, ", "); emit_type(&out, d->params.items[j].type, d->params.items[j].name, &program); }
            str_add(&out, ") {\n");
            emit_stmt_vec(&out, &d->body, &program, 1);
            str_add(&out, "}\n");
        }
    }
    return out.data;
}
