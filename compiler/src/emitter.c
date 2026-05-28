#include "emitter.h"

#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static bool struct_exists(DeclVec *program, const char *name) {
    for (int i = 0; i < program->count; i++) if (program->items[i]->kind == DC_STRUCT && strcmp(program->items[i]->name, name) == 0) return true;
    return false;
}

static const char *fat_type_tag(Type *inner) {
    if (inner->kind == TY_NAME) return inner->name;
    if (inner->kind == TY_RAWPTR) return "ptr";
    return "x";
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
        str_printf(out, "tc_fat_%s", fat_type_tag(t->inner));
        if (name && name[0]) str_printf(out, " %s", name);
        return;
    }
    if (t->kind == TY_FNPTR) {
        // Emit: rettype (*name)(param, param, ...)
        Str decl = {0};
        // Build (*name)(params) part
        if (name && name[0]) str_printf(&decl, "(*%s)", name);
        else str_add(&decl, "(*)");
        str_add(&decl, "(");
        for (int i = 0; i < t->params.count; i++) {
            if (i) str_add(&decl, ", ");
            Str param_str = {0};
            emit_type(&param_str, t->params.items[i].type, "", program);
            str_add(&decl, param_str.data);
            free(param_str.data);
        }
        str_add(&decl, ")");
        // Emit return type with the full declarator as the name
        emit_type(out, t->inner, decl.data, program);
        free(decl.data);
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
            if (!strcmp(e->text, "p++")) { str_add(out, "("); emit_expr(out, e->left, program); str_add(out, "++)"); }
            else if (!strcmp(e->text, "p--")) { str_add(out, "("); emit_expr(out, e->left, program); str_add(out, "--)"); }
            else { str_printf(out, "(%s", !strcmp(e->text, "->") ? "*" : !strcmp(e->text, "@") ? "&" : e->text); emit_expr(out, e->left, program); str_add(out, ")"); }
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
            if (!strcmp(e->text, "cast") && e->args.count == 2 && e->args.items[1]->kind == EX_TYPE) {
                str_add(out, "(("); emit_type(out, e->args.items[1]->type, "", program); str_add(out, ")("); emit_expr(out, e->args.items[0], program); str_add(out, "))");
                break;
            }
            if (!strcmp(e->text, "free") && e->args.count == 1) {
                str_add(out, "free("); emit_expr(out, e->args.items[0], program); str_add(out, ")");
                break;
            }
            if (!strcmp(e->text, "lenof") && e->args.count == 1) {
                str_add(out, "TC_LENOF("); emit_expr(out, e->args.items[0], program); str_add(out, ")");
                break;
            }
            str_printf(out, "%s(", e->text);
            for (int i = 0; i < e->args.count; i++) { if (i) str_add(out, ", "); emit_expr(out, e->args.items[i], program); }
            str_add(out, ")");
            break;
        case EX_INDEX: emit_expr(out, e->left, program); str_add(out, "["); emit_expr(out, e->right, program); str_add(out, "]"); break;
        case EX_FIELD: emit_expr(out, e->left, program); str_printf(out, ".%s", e->text); break;
        case EX_SLICE: str_add(out, "{ .ptr = &"); emit_expr(out, e->left, program); str_add(out, "["); emit_expr(out, e->right, program); str_add(out, "], .len = ("); emit_expr(out, e->third, program); str_add(out, " - "); emit_expr(out, e->right, program); str_add(out, ") }"); break;
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
            if (s->expr) {
                if (s->type->kind == TY_FATPTR && s->expr->kind == EX_UNARY && !strcmp(s->expr->text, "@")) {
                    str_add(out, " = { .ptr = ");
                    emit_expr(out, s->expr->left, program);
                    str_add(out, ", .len = TC_LENOF(");
                    emit_expr(out, s->expr->left, program);
                    str_add(out, ") }");
                } else {
                    str_add(out, " = "); emit_expr(out, s->expr, program);
                }
            }
            else str_add(out, " = {0}");
            str_add(out, ";\n");
            break;
        }
        case ST_IF:
            str_add(out, "if ("); emit_expr(out, s->expr, program); str_add(out, ") {\n");
            emit_stmt_vec(out, &s->body, program, indent + 1);
            emit_indent(out, indent); str_add(out, "}");
            for (int i = 0; i < s->elseifs.count; i++) {
                str_add(out, " else if ("); emit_expr(out, s->elseifs.items[i].cond, program); str_add(out, ") {\n");
                emit_stmt_vec(out, &s->elseifs.items[i].body, program, indent + 1);
                emit_indent(out, indent); str_add(out, "}");
            }
            if (s->else_body.count) {
                str_add(out, " else {\n");
                emit_stmt_vec(out, &s->else_body, program, indent + 1);
                emit_indent(out, indent); str_add(out, "}");
            }
            str_add(out, "\n");
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
        case ST_MATCH: {
            str_add(out, "switch ("); emit_expr(out, s->expr, program); str_add(out, ") {\n");
            for (int i = 0; i < s->arms.count; i++) {
                MatchArm *arm = &s->arms.items[i];
                emit_indent(out, indent + 1);
                if (arm->pattern == NULL) {
                    str_add(out, "default:\n");
                } else {
                    str_add(out, "case "); emit_expr(out, arm->pattern, program); str_add(out, ":\n");
                }
                emit_stmt_vec(out, &arm->body, program, indent + 2);
                emit_indent(out, indent + 2); str_add(out, "break;\n");
            }
            emit_indent(out, indent); str_add(out, "}\n");
            break;
        }
    }
}

static void emit_stmt_vec(Str *out, StmtVec *body, DeclVec *program, int indent) {
    for (int i = 0; i < body->count; i++) if (body->items[i]->kind != ST_DEFER) emit_stmt(out, body->items[i], body, program, indent);
}

static void emit_stmt_vec_with_defers(Str *out, StmtVec *body, DeclVec *program, int indent) {
    for (int i = 0; i < body->count; i++) if (body->items[i]->kind != ST_DEFER) emit_stmt(out, body->items[i], body, program, indent);
    emit_defers(out, body, program, indent);
}

static void collect_fat_types(Type *t, Str *out, DeclVec *program, char **seen, int *seen_count) {
    if (!t) return;
    if (t->kind == TY_FATPTR) {
        const char *tag = fat_type_tag(t->inner);
        for (int i = 0; i < *seen_count; i++) if (!strcmp(seen[i], tag)) return;
        seen[*seen_count] = xstrdup(tag);
        (*seen_count)++;
        Str base = {0};
        emit_type(&base, t->inner, "", program);
        str_printf(out, "typedef struct { %s *ptr; size_t len; } tc_fat_%s;\n", base.data, tag);
        collect_fat_types(t->inner, out, program, seen, seen_count);
    } else if (t->kind == TY_RAWPTR || t->kind == TY_ARRAY) {
        collect_fat_types(t->inner, out, program, seen, seen_count);
    } else if (t->kind == TY_FNPTR) {
        collect_fat_types(t->inner, out, program, seen, seen_count);
        for (int i = 0; i < t->params.count; i++)
            collect_fat_types(t->params.items[i].type, out, program, seen, seen_count);
    }
}

static void scan_fat_types_in_stmts(StmtVec *body, Str *out, DeclVec *program, char **seen, int *seen_count);

static void scan_fat_types_in_type(Type *t, Str *out, DeclVec *program, char **seen, int *seen_count) {
    if (!t) return;
    collect_fat_types(t, out, program, seen, seen_count);
    if (t->inner) scan_fat_types_in_type(t->inner, out, program, seen, seen_count);
    if (t->kind == TY_FNPTR)
        for (int i = 0; i < t->params.count; i++)
            scan_fat_types_in_type(t->params.items[i].type, out, program, seen, seen_count);
}

static void scan_fat_types_in_stmts(StmtVec *body, Str *out, DeclVec *program, char **seen, int *seen_count) {
    for (int i = 0; i < body->count; i++) {
        Stmt *s = body->items[i];
        if (s->type) scan_fat_types_in_type(s->type, out, program, seen, seen_count);
        if (s->body.count) scan_fat_types_in_stmts(&s->body, out, program, seen, seen_count);
    }
}

char *emit_program(DeclVec program) {
    Str out = {0};
    str_add(&out, "#include <stdint.h>\n#include <stddef.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <math.h>\n");
    str_add(&out, "#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))\n");
    str_add(&out, "#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))\n");
    str_add(&out, "#define TC_FAT_LENOF(x) ((x).len)\n");
    str_add(&out, "\n");
    char *seen[64] = {0};
    int seen_count = 0;
    for (int i = 0; i < program.count; i++) {
        Decl *d = program.items[i];
        if (d->type) scan_fat_types_in_type(d->type, &out, &program, seen, &seen_count);
        for (int j = 0; j < d->params.count; j++) scan_fat_types_in_type(d->params.items[j].type, &out, &program, seen, &seen_count);
        if (d->body.count) scan_fat_types_in_stmts(&d->body, &out, &program, seen, &seen_count);
    }
    if (seen_count) str_add(&out, "\n");
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
            if (d->packed) {
                str_printf(&out, "struct __attribute__((packed)) %s {\n", d->name);
            } else {
                str_printf(&out, "struct %s {\n", d->name);
            }
            
            // Track if we're in a union block
            bool in_union_block = false;
            int padding_count = 0;
            
            for (int j = 0; j < d->params.count; j++) {
                Param *field = &d->params.items[j];
                
                // Anonymous padding field - close any open union block
                if (field->is_anonymous) {
                    if (in_union_block) {
                        str_add(&out, "    };\n");
                        in_union_block = false;
                    }
                    // Emit anonymous padding field with unique name
                    char padding_name[32];
                    sprintf(padding_name, "_padding%d", padding_count++);
                    str_printf(&out, "    "); emit_type(&out, field->type, padding_name, &program); str_add(&out, ";\n");
                    continue;
                }
                
                // Regular field (not union)
                if (!field->is_union_field) {
                    if (in_union_block) {
                        str_add(&out, "    };\n");
                        in_union_block = false;
                    }
                    str_add(&out, "    "); emit_type(&out, field->type, field->name, &program); str_add(&out, ";\n");
                } 
                // Union field
                else {
                    if (!in_union_block) {
                        str_add(&out, "    union {\n");
                        in_union_block = true;
                    }
                    str_add(&out, "        "); emit_type(&out, field->type, field->name, &program); str_add(&out, ";\n");
                }
            }
            
            // Close any open union block
            if (in_union_block) {
                str_add(&out, "    };\n");
            }
            
            str_add(&out, "};\n\n");
        }
    }
    for (int i = 0; i < program.count; i++) {
        Decl *d = program.items[i];
        if (d->kind == DC_FN) {
            bool is_main_args = (strcmp(d->name, "main") == 0 && d->params.count == 1 && d->params.items[0].type->kind == TY_FATPTR);
            if (is_main_args) {
                str_add(&out, "int32_t main(int argc, char **argv);\n");
            } else {
                emit_type(&out, d->type, d->name, &program); str_add(&out, "(");
                if (!d->params.count && !d->varargs) str_add(&out, "void");
                for (int j = 0; j < d->params.count; j++) { if (j) str_add(&out, ", "); emit_type(&out, d->params.items[j].type, d->params.items[j].name, &program); }
                if (d->varargs) str_add(&out, d->params.count ? ", ..." : "...");
                str_add(&out, ");\n");
            }
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
            bool is_main_args = (strcmp(d->name, "main") == 0 && d->params.count == 1 && d->params.items[0].type->kind == TY_FATPTR);
            if (is_main_args) {
                str_add(&out, "\nint32_t main(int argc, char **argv) {\n");
                str_printf(&out, "    tc_fat_ptr %s = { .ptr = argv, .len = (size_t)argc };\n", d->params.items[0].name);
            } else {
                str_add(&out, "\n"); emit_type(&out, d->type, d->name, &program); str_add(&out, "(");
                if (!d->params.count) str_add(&out, "void");
                for (int j = 0; j < d->params.count; j++) { if (j) str_add(&out, ", "); emit_type(&out, d->params.items[j].type, d->params.items[j].name, &program); }
                str_add(&out, ") {\n");
            }
            emit_stmt_vec_with_defers(&out, &d->body, &program, 1);
            str_add(&out, "}\n");
        }
    }
    return out.data;
}
