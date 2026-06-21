#include "emitter.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



static bool struct_exists(DeclVec *program, const char *name) {

    for (int i = 0; i < program->count; i++) if (program->items[i]->kind == DC_STRUCT && strcmp(program->items[i]->name, name) == 0) return true;

    return false;

}



static bool type_needs_runtime(Type *t) {

    if (!t) return false;

    // if (t->kind == TY_QUEUE || t->kind == TY_STACK) return true; // This is wrong, since q's and stacks are just data structures, they dont need runtime

    if (t->kind == TY_RAWPTR || t->kind == TY_FATPTR || t->kind == TY_ARRAY || t->kind == TY_FNPTR) {

        if (type_needs_runtime(t->inner)) return true;

    }

    if (t->kind == TY_FNPTR) {

        for (int i = 0; i < t->params.count; i++) {

            if (type_needs_runtime(t->params.items[i].type)) return true;

        }

    }

    return false;

}



static bool expr_needs_runtime(Expr *e) {

    if (!e) return false;

    if (e->kind == EX_QUEUE_METHOD) return true;

    if (e->kind == EX_METHOD_CALL && e->left && e->left->type && (e->left->type->kind == TY_QUEUE || e->left->type->kind == TY_STACK)) {

        if (!strcmp(e->text, "push") || !strcmp(e->text, "enq") || !strcmp(e->text, "pop") || !strcmp(e->text, "deq") || !strcmp(e->text, "peek")) return true;

    }

    if (e->kind == EX_CALL && (!strcmp(e->text, "queue_create") || !strcmp(e->text, "stack_create"))) return true;

    if (expr_needs_runtime(e->left)) return true;

    if (expr_needs_runtime(e->right)) return true;

    if (expr_needs_runtime(e->third)) return true;

    for (int i = 0; i < e->args.count; i++) if (expr_needs_runtime(e->args.items[i])) return true;

    return false;

}



static bool stmt_needs_runtime(Stmt *s) {

    if (!s) return false;

    if (s->type && type_needs_runtime(s->type)) return true;

    if (s->expr && expr_needs_runtime(s->expr)) return true;

    if (s->expr2 && expr_needs_runtime(s->expr2)) return true;

    for (int i = 0; i < s->body.count; i++) if (stmt_needs_runtime(s->body.items[i])) return true;

    for (int i = 0; i < s->elseifs.count; i++) {

        if (expr_needs_runtime(s->elseifs.items[i].cond)) return true;

        for (int j = 0; j < s->elseifs.items[i].body.count; j++) if (stmt_needs_runtime(s->elseifs.items[i].body.items[j])) return true;

    }

    for (int i = 0; i < s->else_body.count; i++) if (stmt_needs_runtime(s->else_body.items[i])) return true;

    return false;

}



static bool decl_needs_runtime(Decl *d) {

    if (!d) return false;

    if (d->type && type_needs_runtime(d->type)) return true;

    if (d->kind == DC_FN) {

        for (int i = 0; i < d->params.count; i++) if (type_needs_runtime(d->params.items[i].type)) return true;

        for (int i = 0; i < d->body.count; i++) if (stmt_needs_runtime(d->body.items[i])) return true;

    }

    return false;

}



static bool type_is_pointer_like(Type *t) {

    return t && (t->kind == TY_RAWPTR || t->kind == TY_FATPTR);

}



static Decl *current_fn = NULL;



static bool async_param_is_queue_or_stack(const char *name, bool *is_stack) {

    if (!current_fn || !current_fn->is_async || !name) return false;

    for (int i = 0; i < current_fn->params.count; i++) {

        Param *param = &current_fn->params.items[i];

        if (param->name && !strcmp(param->name, name) && param->type &&

            (param->type->kind == TY_QUEUE || param->type->kind == TY_STACK)) {

            if (is_stack) *is_stack = param->type->kind == TY_STACK;

            return true;

        }

    }

    return false;

}



static const char *fat_type_tag(Type *inner) {

    if (inner->kind == TY_NAME) return inner->name;

    if (inner->kind == TY_RAWPTR) return "ptr";

    return "x";

}



static void emit_expr(Str *out, Expr *e, DeclVec *program);

static void emit_type(Str *out, Type *t, const char *name, DeclVec *program);



static void emit_type_for_async_param(Str *out, Type *t, const char *name, DeclVec *program) {

    // Queue/stack types should be passed by value, not by pointer

    emit_type(out, t, name, program);

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

    if (t->kind == TY_QUEUE) {

        str_add(out, "Queue");

        if (name && name[0]) str_printf(out, " %s", name);

        return;

    }

    if (t->kind == TY_STACK) {

        str_add(out, "Stack");

        if (name && name[0]) str_printf(out, " %s", name);

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

            else if (!strcmp(e->text, "&ref")) { str_add(out, "&"); emit_expr(out, e->left, program); }

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

            // Queue methods

            if (!strcmp(e->text, "size") && e->args.count == 1) {

                Type *arg_type = e->args.items[0]->type;

                if (arg_type && arg_type->kind == TY_QUEUE) {

                    str_add(out, "queue_size("); emit_expr(out, e->args.items[0], program); str_add(out, ")");

                    break;

                }

                if (arg_type && arg_type->kind == TY_STACK) {

                    str_add(out, "stack_size("); emit_expr(out, e->args.items[0], program); str_add(out, ")");

                    break;

                }

            }

            if (!strcmp(e->text, "clear") && e->args.count == 1) {

                Type *arg_type = e->args.items[0]->type;

                if (arg_type && arg_type->kind == TY_QUEUE) {

                    str_add(out, "queue_clear("); emit_expr(out, e->args.items[0], program); str_add(out, ")");

                    break;

                }

                if (arg_type && arg_type->kind == TY_STACK) {

                    str_add(out, "stack_clear("); emit_expr(out, e->args.items[0], program); str_add(out, ")");

                    break;

                }

            }

            if (!strcmp(e->text, "isEmpty") && e->args.count == 1) {

                Type *arg_type = e->args.items[0]->type;

                if (arg_type && arg_type->kind == TY_QUEUE) {

                    str_add(out, "queue_isEmpty("); emit_expr(out, e->args.items[0], program); str_add(out, ")");

                    break;

                }

                if (arg_type && arg_type->kind == TY_STACK) {

                    str_add(out, "stack_isEmpty("); emit_expr(out, e->args.items[0], program); str_add(out, ")");

                    break;

                }

            }

            // Check if this is an async function call

            bool is_async_call = false;

            for (int i = 0; i < program->count; i++) {

                Decl *d = program->items[i];

                if (d->kind == DC_FN && strcmp(d->name, e->text) == 0 && d->is_async) {

                    is_async_call = true;

                    break;

                }

            }

            

            // Additional check: ensure we have the right function signature

            if (is_async_call) {

                // Double-check this is really an async function

                is_async_call = false;

                for (int i = 0; i < program->count; i++) {

                    Decl *d = program->items[i];

                    if (d->kind == DC_FN && strcmp(d->name, e->text) == 0 && d->is_async) {

                        is_async_call = true;

                        break;

                    }

                }

            }

            

            if (is_async_call) {

                // Generate async call: submit to thread pool

                if (e->args.count == 0) {

                    // No arguments - simple call

                    str_printf(out, "thread_pool_submit(g_thread_pool, (void(*)(void*))%s, NULL);", e->text);

                } else {

                    // Pack arguments into a struct if needed

                    if (e->args.count == 1) {

                        Type *arg_type = e->args.items[0]->type;

                        if (!arg_type) {

                            for (int j = 0; j < program->count; j++) {

                                Decl *d2 = program->items[j];

                                if (d2->kind == DC_FN && strcmp(d2->name, e->text) == 0 && d2->is_async && d2->params.count == 1) {

                                    arg_type = d2->params.items[0].type;

                                    break;

                                }

                            }

                        }

                        bool is_queue_stack = arg_type && (arg_type->kind == TY_QUEUE || arg_type->kind == TY_STACK);

                        bool is_pointer = type_is_pointer_like(arg_type);



                        if (is_queue_stack || is_pointer) {

                            // Queue/stack handles and raw pointers may be passed directly

                            str_printf(out, "thread_pool_submit(g_thread_pool, (void(*)(void*))%s, ", e->text);

                            if (is_queue_stack) {

                                str_add(out, "&");

                            }

                            emit_expr(out, e->args.items[0], program);

                            str_add(out, ");\n");

                        } else {

                            // Primitive types - copy by value using int32_t for simplicity

                            str_add(out, "int32_t *arg = malloc(sizeof(int32_t));\n");

                            str_add(out, "*arg = ");

                            emit_expr(out, e->args.items[0], program);

                            str_add(out, ";\n");

                            str_printf(out, "thread_pool_submit(g_thread_pool, (void(*)(void*))%s_wrapper, arg);", e->text);

                        }

                    } else {

                        // Multiple args - pack them into an allocated struct and submit the wrapper

                        Decl *async_decl = NULL;

                        for (int k = 0; k < program->count; k++) {

                            Decl *d2 = program->items[k];

                            if (d2->kind == DC_FN && strcmp(d2->name, e->text) == 0 && d2->is_async) {

                                async_decl = d2;

                                break;

                            }

                        }

                        str_printf(out, "struct %s_args {", e->text);

                        for (int j = 0; j < e->args.count; j++) {

                            Type *arg_type = e->args.items[j]->type;

                            if (!arg_type && async_decl && j < async_decl->params.count) arg_type = async_decl->params.items[j].type;

                            char field_name[16];

                            sprintf(field_name, "arg%d", j);

                            if (arg_type) {

                                emit_type_for_async_param(out, arg_type, field_name, program);

                            } else {

                                str_printf(out, "int32_t %s", field_name);

                            }

                            str_add(out, "; ");

                        }

                        str_add(out, "};\n");

                        str_printf(out, "struct %s_args *arg = malloc(sizeof(struct %s_args));\n", e->text, e->text);

                        for (int j = 0; j < e->args.count; j++) {

                            Type *arg_type = e->args.items[j]->type;

                            if (!arg_type && async_decl && j < async_decl->params.count) arg_type = async_decl->params.items[j].type;

                            char field_name[16];

                            sprintf(field_name, "arg%d", j);

                            str_printf(out, "arg->%s = ", field_name);

                            if (arg_type && (arg_type->kind == TY_QUEUE || arg_type->kind == TY_STACK)) {

                                str_add(out, "&");

                            }

                            emit_expr(out, e->args.items[j], program);

                            str_add(out, ";\n");

                        }

                        str_printf(out, "thread_pool_submit(g_thread_pool, (void(*)(void*))%s_wrapper, arg);", e->text);

                    }

                }

            } else {

                str_printf(out, "%s(", e->text);

                for (int i = 0; i < e->args.count; i++) {

                    if (e->args.items[i]->kind == EX_VARARGS) continue;  // Skip varargs marker in calls

                    if (i) str_add(out, ", ");

                    // Check for pass-by-reference marker

                    if (e->args.items[i]->kind == EX_UNARY && !strcmp(e->args.items[i]->text, "&ref")) {

                        str_add(out, "&");

                        emit_expr(out, e->args.items[i]->left, program);

                    } else {

                        emit_expr(out, e->args.items[i], program);

                    }

                }

                str_add(out, ")");

            }

            break;

        case EX_QUEUE_METHOD: {

            // Generate queue/stack method calls

            Type *inner = NULL;
            if (e->left && e->left->type) {
                if (e->left->type->kind == TY_RAWPTR && e->left->type->inner) {
                    // Pointer to queue/stack - get the item type from the queue/stack's inner
                    if (e->left->type->inner->kind == TY_QUEUE || e->left->type->inner->kind == TY_STACK) {
                        inner = e->left->type->inner->inner;
                    } else {
                        inner = e->left->type->inner;
                    }
                } else {
                    // Direct queue/stack - get the item type
                    inner = e->left->type->inner;
                }
            }

            bool is_stack = e->left && e->left->type && (e->left->type->kind == TY_STACK || (e->left->type->kind == TY_RAWPTR && e->left->type->inner && e->left->type->inner->kind == TY_STACK));

            Str item_type = {0};

            if (inner) emit_type(&item_type, inner, "", program);

            else str_add(&item_type, "void");



            bool left_is_ptr = e->left && e->left->type && e->left->type->kind == TY_RAWPTR && e->left->type->inner && (e->left->type->inner->kind == TY_QUEUE || e->left->type->inner->kind == TY_STACK);

            if (!left_is_ptr && e->left && e->left->kind == EX_NAME) {

                bool async_stack = false;

                if (async_param_is_queue_or_stack(e->left->text, &async_stack)) {

                    left_is_ptr = true;

                    if (async_stack) is_stack = true;

                }

            }

            // Always take address for C API since functions expect pointers

            const char *addr = left_is_ptr ? "" : "&";

            if (!strcmp(e->text, "push") || !strcmp(e->text, "enq")) {

                str_printf(out, "%s_push(%s", is_stack ? "stack" : "queue", addr);

                emit_expr(out, e->left, program);

                str_add(out, ", &(");

                str_add(out, item_type.data);

                str_add(out, "){ ");

                emit_expr(out, e->args.items[0], program);

                str_add(out, " }, sizeof(");

                str_add(out, item_type.data);

                str_add(out, "))");

            } else if (!strcmp(e->text, "pop") || !strcmp(e->text, "deq") || !strcmp(e->text, "peek")) {

                str_add(out, "*(");

                str_add(out, item_type.data);

                str_add(out, "*)");

                if (!strcmp(e->text, "peek")) {

                    str_printf(out, "%s_peek(%s", is_stack ? "stack" : "queue", addr);

                } else {

                    str_printf(out, "%s_pop(%s", is_stack ? "stack" : "queue", addr);

                }

                emit_expr(out, e->left, program);

                str_add(out, ")");

            } else if (!strcmp(e->text, "size")) {

                str_printf(out, "%s_size(%s", is_stack ? "stack" : "queue", addr);

                emit_expr(out, e->left, program);

                str_add(out, ")");

            } else if (!strcmp(e->text, "clear")) {

                str_printf(out, "%s_clear(%s", is_stack ? "stack" : "queue", addr);

                emit_expr(out, e->left, program);

                str_add(out, ")");

            } else if (!strcmp(e->text, "isEmpty") || !strcmp(e->text, "is_empty")) {

                str_printf(out, "%s_isEmpty(%s", is_stack ? "stack" : "queue", addr);

                emit_expr(out, e->left, program);

                str_add(out, ")");

            }

            free(item_type.data);

            break;

        }

        case EX_INDEX: emit_expr(out, e->left, program); str_add(out, "["); emit_expr(out, e->right, program); str_add(out, "]"); break;

        case EX_FIELD: emit_expr(out, e->left, program); str_printf(out, ".%s", e->text); break;

        case EX_PTR_FIELD: emit_expr(out, e->left, program); str_printf(out, "->%s", e->text); break;

        case EX_METHOD_CALL:

            // Emit method call as StructName_methodName(&expr, args...)

            // Check if this is a queue/stack method call

            if (e->left && e->left->type && (e->left->type->kind == TY_QUEUE || e->left->type->kind == TY_STACK)) {

                // Queue/stack method call

                bool is_stack = e->left->type->kind == TY_STACK;

                Str item_type = {0};

                if (e->left->type->inner) emit_type(&item_type, e->left->type->inner, "", program);

                else str_add(&item_type, "void");



                // Check if left side is a pointer (raw pointer to queue/stack)

                // C functions always expect pointers, so add & if not already a pointer

                bool left_is_ptr = (e->left->type->kind == TY_RAWPTR && e->left->type->inner && 

                                  (e->left->type->inner->kind == TY_QUEUE || e->left->type->inner->kind == TY_STACK));

                

                if (!strcmp(e->text, "push") || !strcmp(e->text, "enq")) {

                    str_printf(out, "%s_push(%s", is_stack ? "stack" : "queue", left_is_ptr ? "" : "&");

                    emit_expr(out, e->left, program);

                    str_add(out, ", &(");

                    str_add(out, item_type.data);

                    str_add(out, "){ ");

                    emit_expr(out, e->args.items[0], program);

                    str_add(out, " }, sizeof(");

                    str_add(out, item_type.data);

                    str_add(out, "))");

                } else if (!strcmp(e->text, "pop") || !strcmp(e->text, "deq") || !strcmp(e->text, "peek")) {

                    str_add(out, "*(");

                    str_add(out, item_type.data);

                    str_add(out, "*)");

                    if (!strcmp(e->text, "peek")) {

                        str_printf(out, "%s_peek(%s", is_stack ? "stack" : "queue", left_is_ptr ? "" : "&");

                    } else {

                        str_printf(out, "%s_pop(%s", is_stack ? "stack" : "queue", left_is_ptr ? "" : "&");

                    }

                    emit_expr(out, e->left, program);

                    str_add(out, ")");

                } else if (!strcmp(e->text, "size")) {

                    str_printf(out, "%s_size(%s", is_stack ? "stack" : "queue", left_is_ptr ? "" : "&");

                    emit_expr(out, e->left, program);

                    str_add(out, ")");

                } else if (!strcmp(e->text, "clear")) {

                    str_printf(out, "%s_clear(%s", is_stack ? "stack" : "queue", left_is_ptr ? "" : "&");

                    emit_expr(out, e->left, program);

                    str_add(out, ")");

                } else if (!strcmp(e->text, "isEmpty")) {

                    str_printf(out, "%s_isEmpty(%s", is_stack ? "stack" : "queue", left_is_ptr ? "" : "&");

                    emit_expr(out, e->left, program);

                    str_add(out, ")");

                } else {

                    // Unknown method, fall back to regular emission

                    if (e->type && e->type->name) {

                        str_printf(out, "%s_%s(&", e->type->name, e->text);

                    } else {

                        str_add(out, e->text);

                        str_add(out, "(&");

                    }

                    emit_expr(out, e->left, program);

                    for (int i = 0; i < e->args.count; i++) {

                        str_add(out, ", ");

                        emit_expr(out, e->args.items[i], program);

                    }

                    str_add(out, ")");

                }

                free(item_type.data);

                break;

            }

            

            // Regular struct method call

            if (e->type && e->type->name) {

                str_printf(out, "%s_%s(&", e->type->name, e->text);

            } else {

                // Fallback if struct name not available

                str_add(out, e->text);

                str_add(out, "(&");

            }

            emit_expr(out, e->left, program);

            for (int i = 0; i < e->args.count; i++) {

                str_add(out, ", ");

                emit_expr(out, e->args.items[i], program);

            }

            str_add(out, ")");

            break;

        case EX_SLICE: str_add(out, "{ .ptr = &"); emit_expr(out, e->left, program); str_add(out, "["); emit_expr(out, e->right, program); str_add(out, "], .len = ("); emit_expr(out, e->third, program); str_add(out, " - "); emit_expr(out, e->right, program); str_add(out, ") }"); break;

        case EX_INIT_LIST:

            str_add(out, "{");

            for (int i = 0; i < e->args.count; i++) { if (i) str_add(out, ", "); emit_expr(out, e->args.items[i], program); }

            str_add(out, "}");

            break;

        case EX_TYPE: emit_type(out, e->type, "", program); break;

        case EX_VARARGS: break;  // Don't emit ... in function calls - it's only for declarations

        case EX_SIZEOF:

            str_add(out, "sizeof(");

            if (e->left && e->left->type) {

                emit_type(out, e->left->type, "", program);

            } else {

                str_add(out, "int");  // fallback

            }

            str_add(out, ")");

            break;

        case EX_CAST:

            str_add(out, "(");

            emit_type(out, e->cast_type, "", program);

            str_add(out, ")");

            emit_expr(out, e->left, program);

            break;

    }

}



static void emit_indent(Str *out, int indent) {

    for (int i = 0; i < indent; i++) str_add(out, "    ");

}



static void emit_stmt_vec(Str *out, StmtVec *body, DeclVec *program, int indent);

static bool emit_stmt_vec_with_defers(Str *out, StmtVec *body, DeclVec *program, int indent);



static void emit_defers(Str *out, StmtVec *body, DeclVec *program, int indent) {

    for (int i = body->count - 1; i >= 0; i--) {

        Stmt *s = body->items[i];

        if (s->kind == ST_DEFER) emit_stmt_vec(out, &s->body, program, indent);

    }

}



static void emit_stmt(Str *out, Stmt *s, StmtVec *scope, DeclVec *program, int indent, bool *has_return) {

    emit_indent(out, indent);

    switch (s->kind) {

        case ST_VAR: {

            if ((s->type->kind == TY_QUEUE || s->type->kind == TY_STACK) && s->expr && s->expr->kind == EX_INIT_LIST) {

                Type *inner = s->type->inner;

                Str item_type = {0};

                if (inner) emit_type(&item_type, inner, "", program);

                else str_add(&item_type, "void");



                bool is_stack = s->type->kind == TY_STACK;

                const char *container = is_stack ? "stack" : "queue";

                str_printf(out, "%s %s = %s_create(%d);\n", is_stack ? "Stack" : "Queue", s->name, container, s->expr->args.count);

                for (int i = 0; i < s->expr->args.count; i++) {

                    emit_indent(out, indent);

                    str_printf(out, "%s_push(&%s, &(", container, s->name);

                    str_add(out, item_type.data);

                    str_add(out, "){ ");

                    emit_expr(out, s->expr->args.items[i], program);

                    str_add(out, " }, sizeof(");

                    str_add(out, item_type.data);

                    str_add(out, "));\n");

                }

                free(item_type.data);

            } else {

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

                } else if (s->type->kind == TY_QUEUE || s->type->kind == TY_STACK) {

                    const char *container = s->type->kind == TY_STACK ? "stack" : "queue";

                    str_printf(out, " = %s_create(0)", container);

                } else {

                    str_add(out, " = {0}");

                }

                str_add(out, ";\n");

            }

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

            if (has_return) *has_return = true;

            break;

        case ST_BREAK: str_add(out, "break;\n"); break;

        case ST_PIN:

            // Pin statement - no runtime code needed, just a comment for debugging

            str_add(out, "/* pin "); emit_expr(out, s->expr, program); str_add(out, " */\n");

            break;

        case ST_SELECT:

            // Generate select statement as a simple switch for now

            str_add(out, "// TODO: Implement proper select statement\n");

            str_add(out, "switch (0) {\n");

            // For now, just emit the first case as default

            if (s->body.count > 0) {

                emit_indent(out, indent + 1);

                str_add(out, "default:\n");

                emit_stmt_vec(out, &s->body, program, indent + 2);

                emit_indent(out, indent + 2);

                str_add(out, "break;\n");

            }

            emit_indent(out, indent);

            str_add(out, "}\n");

            break;

        case ST_EXPR: emit_expr(out, s->expr, program); str_add(out, ";\n"); break;

        case ST_INLINE_C:

            // Emit inline C code directly

            str_add(out, "\n");

            str_add(out, s->text);

            str_add(out, "\n");

            break;

        case ST_DEFER: break;

        case ST_THROW: {
            // Generate throw as: call error body, then tc_throw with error name
            if (s->expr && s->expr->text) {
                // First, emit the error body function call
                str_add(out, "error_");
                str_add(out, s->expr->text);
                str_add(out, "(");
                if (s->expr->args.count > 0) {
                    for (int i = 0; i < s->expr->args.count; i++) {
                        emit_expr(out, s->expr->args.items[i], program);
                        if (i < s->expr->args.count - 1) str_add(out, ", ");
                    }
                }
                str_add(out, ");\n");
                // Then, emit tc_throw with the error name
                emit_indent(out, indent);
                str_add(out, "tc_throw(\"");
                str_add(out, s->expr->text);
                str_add(out, "\");\n");
            } else {
                // Fallback for other expressions
                str_add(out, "tc_throw(\"unknown\");\n");
            }
            break;
        }

        case ST_TRY: {
            // Generate try-catch using setjmp/longjmp
            str_add(out, "{\n");
            emit_indent(out, indent + 1); str_add(out, "jmp_buf _try_buf;\n");
            emit_indent(out, indent + 1); str_add(out, "jmp_buf *_old_buf = tc_get_try_buf();\n");
            emit_indent(out, indent + 1); str_add(out, "tc_set_try_buf(&_try_buf);\n");
            emit_indent(out, indent + 1); str_add(out, "int _try_result = setjmp(_try_buf);\n");
            emit_indent(out, indent + 1); str_add(out, "if (_try_result == 0) {\n");
            emit_stmt_vec(out, &s->body, program, indent + 2);
            emit_indent(out, indent + 1); str_add(out, "} else {\n");
            // Emit catch arms
            for (int i = 0; i < s->catch_arms.count; i++) {
                CatchArm *arm = &s->catch_arms.items[i];
                if (arm->error_name) {
                    emit_indent(out, indent + 2); str_add(out, "if (strcmp(tc_error_name(), \"");
                    str_add(out, arm->error_name);
                    str_add(out, "\") == 0) {\n");
                    if (arm->ret_expr) {
                        emit_indent(out, indent + 3); str_add(out, "return ");
                        emit_expr(out, arm->ret_expr, program);
                        str_add(out, ";\n");
                    } else if (arm->body.count > 0) {
                        emit_stmt_vec(out, &arm->body, program, indent + 3);
                    }
                    emit_indent(out, indent + 2); str_add(out, "}\n");
                } else {
                    // Wildcard catch
                    emit_indent(out, indent + 2); str_add(out, "else {\n");
                    if (arm->ret_expr) {
                        emit_indent(out, indent + 3); str_add(out, "return ");
                        emit_expr(out, arm->ret_expr, program);
                        str_add(out, ";\n");
                    } else if (arm->body.count > 0) {
                        emit_stmt_vec(out, &arm->body, program, indent + 3);
                    }
                    emit_indent(out, indent + 2); str_add(out, "}\n");
                }
            }
            emit_indent(out, indent + 1); str_add(out, "}\n");
            emit_indent(out, indent + 1); str_add(out, "tc_set_try_buf(_old_buf);\n");
            emit_indent(out, indent); str_add(out, "}\n");
            break;
        }

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

    for (int i = 0; i < body->count; i++) if (body->items[i]->kind != ST_DEFER) emit_stmt(out, body->items[i], body, program, indent, NULL);

}



static bool emit_stmt_vec_with_defers(Str *out, StmtVec *body, DeclVec *program, int indent) {

    bool has_return = false;

    // First pass: collect queue/stack variables that need cleanup

    Str cleanup_code = {0};

    for (int i = 0; i < body->count; i++) {

        Stmt *s = body->items[i];

        if (s->kind == ST_VAR && (s->type->kind == TY_QUEUE || s->type->kind == TY_STACK)) {

            const char *container = s->type->kind == TY_STACK ? "stack" : "queue";

            str_printf(&cleanup_code, "    %s_destroy(&%s);\n", container, s->name);

        }

    }

    

    // Second pass: emit statements

    for (int i = 0; i < body->count; i++) if (body->items[i]->kind != ST_DEFER) emit_stmt(out, body->items[i], body, program, indent, &has_return);

    

    // Emit cleanup for queue/stack variables (before defers, after regular statements)

    if (cleanup_code.len > 0 && !has_return) {

        emit_indent(out, indent);

        str_add(out, "/* cleanup queue/stack */\n");

        str_add(out, cleanup_code.data);

    }

    

    // Emit defers

    if (!has_return) emit_defers(out, body, program, indent);

    

    free(cleanup_code.data);

    return has_return;

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



char *emit_program(DeclVec program, const char *stdlib_path, bool freestanding) {

    Str out = {0};

    if (freestanding) {
        // Minimal type definitions for freestanding mode
        str_add(&out, "// Minimal type definitions for freestanding mode\n");
        str_add(&out, "typedef __INT8_TYPE__ int8_t;\n");
        str_add(&out, "typedef __INT16_TYPE__ int16_t;\n");
        str_add(&out, "typedef __INT32_TYPE__ int32_t;\n");
        str_add(&out, "typedef __INT64_TYPE__ int64_t;\n");
        str_add(&out, "typedef __UINT8_TYPE__ uint8_t;\n");
        str_add(&out, "typedef __UINT16_TYPE__ uint16_t;\n");
        str_add(&out, "typedef __UINT32_TYPE__ uint32_t;\n");
        str_add(&out, "typedef __UINT64_TYPE__ uint64_t;\n");
        str_add(&out, "typedef __INTPTR_TYPE__ intptr_t;\n");
        str_add(&out, "typedef __UINTPTR_TYPE__ uintptr_t;\n");
        str_add(&out, "typedef __SIZE_TYPE__ size_t;\n");
        str_add(&out, "typedef __PTRDIFF_TYPE__ ptrdiff_t;\n");
        str_add(&out, "#define NULL ((void*)0)\n");
        str_add(&out, "#define volatile __volatile__\n");
        str_add(&out, "\n");
    } else {
        str_add(&out, "#include <stdint.h>\n#include <stddef.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <math.h>\n#include <setjmp.h>\n");
    }

    str_add(&out, "#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))\n");

    str_add(&out, "#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))\n");

    str_add(&out, "#define TC_FAT_LENOF(x) ((x).len)\n");

    

    // Check if we need runtime support (async functions, queue/stack types, queue/stack methods, or try/catch)

    bool needs_runtime = false;

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if ((d->kind == DC_FN && d->is_async)) { //|| decl_needs_runtime(d)

            needs_runtime = true;

            break;

        }

        // Check if function uses try/catch
        if (d->kind == DC_FN && d->body.count > 0) {
            for (int j = 0; j < d->body.count; j++) {
                if (d->body.items[j]->kind == ST_TRY || d->body.items[j]->kind == ST_THROW) {
                    needs_runtime = true;
                    break;
                }
            }
            if (needs_runtime) break;
        }

    }

    

    if (needs_runtime) {

        str_printf(&out, "#include \"%s/async.h\"\n", stdlib_path);

        // Add function declarations for error handling
        str_add(&out, "jmp_buf *tc_get_try_buf(void);\n");
        str_add(&out, "void tc_set_try_buf(jmp_buf *buf);\n");
        str_add(&out, "void tc_throw(const char *error_name);\n");
        str_add(&out, "const char *tc_error_name(void);\n");

    }

    

    str_add(&out, "\n");

    // Emit global inline C code (for #include statements, etc.)

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_INLINE_C && d->text) {

            str_add(&out, d->text);

            str_add(&out, "\n");

        }

    }

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

            // Resolve path relative to stdlib_path if it starts with "stdlib/"
            char full_path[512];
            if (strncmp(path, "stdlib/", 7) == 0) {
                snprintf(full_path, sizeof(full_path), "%s/%s", stdlib_path, path + 7);
            } else {
                snprintf(full_path, sizeof(full_path), "%s", path);
            }

            str_printf(&out, "#include \"%s\"\n", full_path);

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

        if (d->kind == DC_ENUM) {

            str_printf(&out, "typedef enum {\n");

            for (int j = 0; j < d->enum_members.count; j++) {

                str_printf(&out, "    %s", d->enum_members.items[j].name);

                if (j < d->enum_members.count - 1) str_add(&out, ",\n");

                else str_add(&out, "\n");

            }

            str_printf(&out, "} %s;\n\n", d->name);

        }

        if (d->kind == DC_ERROR) {

            // Emit error as a function that handles the error

            // Error handlers don't have a return type, they're void

            str_add(&out, "void");

            str_add(&out, " ");

            str_printf(&out, "error_%s(", d->name);

            for (int j = 0; j < d->params.count; j++) {

                if (j) str_add(&out, ", ");

                emit_type(&out, d->params.items[j].type, d->params.items[j].name, &program);

            }

            str_add(&out, ") {\n");

            emit_stmt_vec(&out, &d->body, &program, 1);

            str_add(&out, "}\n\n");

        }

        // Emit method forward declarations for structs

        if (d->kind == DC_STRUCT && d->methods.count > 0) {

            for (int m = 0; m < d->methods.count; m++) {

                Decl *method = d->methods.items[m];

                // Method name: StructName_methodName

                emit_type(&out, method->type, "", &program);

                str_add(&out, " ");

                str_printf(&out, "%s_%s(", d->name, method->name);

                // First parameter is self (pointer to struct)

                str_printf(&out, "struct %s *self", d->name);

                // Then the rest of the parameters

                for (int j = 0; j < method->params.count; j++) {

                    str_add(&out, ", ");

                    emit_type(&out, method->params.items[j].type, method->params.items[j].name, &program);

                }

                if (method->varargs) str_add(&out, ", ...");

                str_add(&out, ");\n");

            }

            str_add(&out, "\n");

        }

    }

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_FN) {

            bool is_main_args = (strcmp(d->name, "main") == 0 && d->params.count == 1 && d->params.items[0].type->kind == TY_FATPTR);

            if (is_main_args) {

                str_add(&out, "int32_t main(int argc, char **argv);\n");

            } else {
                if (d->is_raw) {
                    str_add(&out, "volatile ");
                }
                emit_type(&out, d->type, d->name, &program); str_add(&out, "(");

                if (!d->params.count && !d->varargs) str_add(&out, "void");

                for (int j = 0; j < d->params.count; j++) {

                    if (j) str_add(&out, ", ");

                    emit_type_for_async_param(&out, d->params.items[j].type, d->params.items[j].name, &program);

                }

                if (d->varargs) str_add(&out, d->params.count ? ", ..." : "...");

                str_add(&out, ");\n");

            }

        }

        // Don't emit forward declarations for extern C functions - they're in C headers

    }

    str_add(&out, "\n");

    

    // Generate wrapper functions for async functions that take primitive or multiple arguments

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_FN && d->is_async) {

            if (d->params.count == 1) {

                Type *param_type = d->params.items[0].type;

                if (param_type && param_type->kind != TY_QUEUE && param_type->kind != TY_STACK) {

                    // Generate wrapper function for single primitive arg

                    str_add(&out, "// Wrapper for async function ");

                    str_add(&out, d->name);

                    str_add(&out, "\n");

                    str_add(&out, "static void ");

                    str_add(&out, d->name);

                    str_add(&out, "_wrapper(void *arg) {\n");

                    if (type_is_pointer_like(param_type)) {

                        str_add(&out, "    ");

                        emit_type(&out, param_type, "val", &program);

                        str_add(&out, " = (");

                        emit_type(&out, param_type, "", &program);

                        str_add(&out, ")arg;\n");

                        str_printf(&out, "    %s(val);\n", d->name);

                    } else {

                        str_add(&out, "    ");

                        emit_type(&out, param_type, "val", &program);

                        str_add(&out, " = *(");

                        emit_type(&out, param_type, "", &program);

                        str_add(&out, "*)arg;\n");

                        str_add(&out, "    free(arg);\n");

                        str_printf(&out, "    %s(val);\n", d->name);

                    }

                    str_add(&out, "}\n\n");

                }

            } else if (d->params.count > 1) {

                // Generate wrapper function for multiple arguments

                str_add(&out, "// Wrapper for async function ");

                str_add(&out, d->name);

                str_add(&out, "\n");

                str_printf(&out, "struct %s_args {", d->name);

                for (int j = 0; j < d->params.count; j++) {

                    char field_name[16];

                    sprintf(field_name, "arg%d", j);

                    Type *param_type = d->params.items[j].type;

                    if (param_type) {

                        emit_type_for_async_param(&out, param_type, field_name, &program);

                    } else {

                        str_printf(&out, "int32_t %s", field_name);

                    }

                    str_add(&out, "; ");

                }

                str_add(&out, "};\n");

                str_add(&out, "static void ");

                str_add(&out, d->name);

                str_add(&out, "_wrapper(void *arg) {\n");

                str_add(&out, "    struct ");

                str_add(&out, d->name);

                str_add(&out, "_args *data = arg;\n");

                str_add(&out, "    ");

                str_add(&out, d->name);

                str_add(&out, "(");

                for (int j = 0; j < d->params.count; j++) {

                    if (j) str_add(&out, ", ");

                    char field_name[16];

                    sprintf(field_name, "arg%d", j);

                    str_printf(&out, "data->%s", field_name);

                }

                str_add(&out, ");\n");

                str_add(&out, "    free(arg);\n");

                str_add(&out, "}\n\n");

            }

        }

    }

    

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

                if (d->is_raw) {
                    str_add(&out, "volatile ");
                }
                str_add(&out, "\n"); emit_type(&out, d->type, d->name, &program); str_add(&out, "(");

                if (!d->params.count && !d->varargs) str_add(&out, "void");

                for (int j = 0; j < d->params.count; j++) {

                    if (j) str_add(&out, ", ");

                    emit_type_for_async_param(&out, d->params.items[j].type, d->params.items[j].name, &program);

                }

                if (d->varargs) str_add(&out, d->params.count ? ", ..." : "...");

                str_add(&out, ") {\n");

            }

            current_fn = d;

            emit_stmt_vec_with_defers(&out, &d->body, &program, 1);

            current_fn = NULL;

            str_add(&out, "}\n");

        }

        // Don't emit bodies for extern C functions - they're already in the C library

    }

    // Emit method implementations for structs

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_STRUCT && d->methods.count > 0) {

            for (int m = 0; m < d->methods.count; m++) {

                Decl *method = d->methods.items[m];

                // Method name: StructName_methodName

                str_add(&out, "\n"); emit_type(&out, method->type, "", &program);

                str_add(&out, " ");

                str_printf(&out, "%s_%s(struct %s *self", d->name, method->name, d->name);

                // Then the rest of the parameters

                for (int j = 0; j < method->params.count; j++) {

                    str_add(&out, ", ");

                    emit_type(&out, method->params.items[j].type, method->params.items[j].name, &program);

                }

                if (method->varargs) str_add(&out, ", ...");

                str_add(&out, ") {\n");

                emit_stmt_vec_with_defers(&out, &method->body, &program, 1);

                str_add(&out, "}\n");

            }

        }

    }

    return out.data;

}



char *emit_hot_split(DeclVec program, const char *hot_lib, char **hot_c_out, const char *stdlib_path) {

    Str host = {0};

    Str hot = {0};



    // === HOT LIBRARY: Contains ALL code ===

    str_add(&hot, "#include <stdint.h>\n#include <stddef.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <math.h>\n");

    str_add(&hot, "#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))\n");

    str_add(&hot, "#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))\n");

    str_add(&hot, "#define TC_FAT_LENOF(x) ((x).len)\n\n");



    // Emit includes from @use statements

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_USE) {

            char *raw = d->path;

            if (raw[0] == '"') raw++;

            size_t plen = strlen(raw);

            if (plen > 0 && raw[plen - 1] == '"') plen--;

            char *path = xstrndup(raw, plen);

            if (plen > 3 && strcmp(path + plen - 3, ".tc") == 0) {

                path[plen - 2] = 'h';

                path[plen - 1] = '\0';

            }

            // Resolve path relative to stdlib_path if it starts with "stdlib/"
            char full_path[512];
            if (strncmp(path, "stdlib/", 7) == 0) {
                snprintf(full_path, sizeof(full_path), "%s/%s", stdlib_path, path + 7);
            } else {
                snprintf(full_path, sizeof(full_path), "%s", path);
            }

            str_printf(&hot, "#include \"%s\"\n", full_path);

        }

    }

    str_add(&hot, "\n");



    // Emit fat pointer types

    char *seen_hot[64] = {0};

    int seen_count_hot = 0;

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->type) scan_fat_types_in_type(d->type, &hot, &program, seen_hot, &seen_count_hot);

        for (int j = 0; j < d->params.count; j++) {

            scan_fat_types_in_type(d->params.items[j].type, &hot, &program, seen_hot, &seen_count_hot);

        }

        if (d->body.count) scan_fat_types_in_stmts(&d->body, &hot, &program, seen_hot, &seen_count_hot);

    }

    str_add(&hot, "\n");



    // Emit struct definitions

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_STRUCT) {

            Str struct_def = {0};

            str_add(&struct_def, "typedef struct {\n");

            bool in_union_block = false;

            for (int j = 0; j < d->params.count; j++) {

                Param *field = &d->params.items[j];

                if (field->is_union_field) {

                    if (!in_union_block) {

                        str_add(&struct_def, "    union {\n");

                        in_union_block = true;

                    }

                    str_add(&struct_def, "        "); emit_type(&struct_def, field->type, field->name, &program); str_add(&struct_def, ";\n");

                } else {

                    if (in_union_block) {

                        str_add(&struct_def, "    };\n");

                        in_union_block = false;

                    }

                    str_add(&struct_def, "        "); emit_type(&struct_def, field->type, field->name, &program); str_add(&struct_def, ";\n");

                }

            }

            if (in_union_block) str_add(&struct_def, "    };\n");

            str_add(&struct_def, "} ");

            if (d->packed) str_add(&struct_def, "__attribute__((packed)) ");

            str_printf(&struct_def, "%s;\n\n", d->name);

            str_add(&hot, struct_def.data);

            free(struct_def.data);

        }

    }



    // === HOT LIBRARY: All function bodies ===

    // (Forward declarations not needed - function bodies are definitions)

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_FN) {

            bool is_main = (strcmp(d->name, "main") == 0);

            if (is_main) {

                // Do not emit main into the DLL!

                continue;

            }

            // Regular functions keep their names and are exported from the DLL

            str_add(&hot, "\n#ifdef _WIN32\n__declspec(dllexport)\n#endif\n");

            emit_type(&hot, d->type, d->name, &program); str_add(&hot, "(");

            if (!d->params.count) str_add(&hot, "void");

            for (int j = 0; j < d->params.count; j++) { if (j) str_add(&hot, ", "); emit_type(&hot, d->params.items[j].type, d->params.items[j].name, &program); }

            str_add(&hot, ") {\n");

            emit_stmt_vec_with_defers(&hot, &d->body, &program, 1);

            str_add(&hot, "}\n");

        }

    }



    // === HOST: Stubs that call into hot library ===

    str_add(&host, "#include <stdint.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n");

#ifdef _WIN32

    str_add(&host, "#include <windows.h>\n");

#else

    str_add(&host, "#include <dlfcn.h>\n");

#endif



    str_add(&host, "#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))\n");

    str_add(&host, "#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))\n");

    str_add(&host, "#define TC_FAT_LENOF(x) ((x).len)\n\n");



    // Emit fat pointer types for host

    char *seen_host[64] = {0};

    int seen_count_host = 0;

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->type) scan_fat_types_in_type(d->type, &host, &program, seen_host, &seen_count_host);

        for (int j = 0; j < d->params.count; j++) {

            scan_fat_types_in_type(d->params.items[j].type, &host, &program, seen_host, &seen_count_host);

        }

        if (d->body.count) scan_fat_types_in_stmts(&d->body, &host, &program, seen_host, &seen_count_host);

    }

    str_add(&host, "\n");



    // Emit struct definitions for host

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_STRUCT) {

            Str struct_def = {0};

            str_add(&struct_def, "typedef struct {\n");

            bool in_union_block = false;

            for (int j = 0; j < d->params.count; j++) {

                Param *field = &d->params.items[j];

                if (field->is_union_field) {

                    if (!in_union_block) {

                        str_add(&struct_def, "    union {\n");

                        in_union_block = true;

                    }

                    str_add(&struct_def, "        "); emit_type(&struct_def, field->type, field->name, &program); str_add(&struct_def, ";\n");

                } else {

                    if (in_union_block) {

                        str_add(&struct_def, "    };\n");

                        in_union_block = false;

                    }

                    str_add(&struct_def, "        "); emit_type(&struct_def, field->type, field->name, &program); str_add(&struct_def, ";\n");

                }

            }

            if (in_union_block) str_add(&struct_def, "    };\n");

            str_add(&struct_def, "} ");

            if (d->packed) str_add(&struct_def, "__attribute__((packed)) ");

            str_printf(&struct_def, "%s;\n\n", d->name);

            str_add(&host, struct_def.data);

            free(struct_def.data);

        }

    }



    // === HOST: Function pointer dispatch table ===

    str_add(&host, "// Hot reload function pointer table\n");

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_FN) {

            if (strcmp(d->name, "main") == 0) continue; // Skip main pointer

            emit_type(&host, d->type, "", &program);

            str_printf(&host, "(*hot_%s)(", d->name);

            if (!d->params.count) str_add(&host, "void");

            for (int j = 0; j < d->params.count; j++) {

                if (j) str_add(&host, ", ");

                emit_type(&host, d->params.items[j].type, "", &program);

            }

            str_add(&host, ");\n");

        }

    }

    str_add(&host, "\n");



    // === HOST: Library management ===

#ifdef _WIN32

    str_add(&host, "static HMODULE hot_lib_handle = NULL;\n");

#else

    str_add(&host, "static void *hot_lib_handle = NULL;\n");

#endif

    str_add(&host, "static int current_version = 0;\n\n");



    str_add(&host, "static int get_version(void) {\n");

    str_printf(&host, "    FILE *vf = fopen(\"%s.version\", \"r\");\n", hot_lib);

    str_add(&host, "    int version = 1;\n");

    str_add(&host, "    if (vf) { fscanf(vf, \"%d\", &version); fclose(vf); }\n");

    str_add(&host, "    return version;\n");

    str_add(&host, "}\n\n");



    str_add(&host, "static void load_hot_functions(int version) {\n");

    str_add(&host, "    if (version == current_version) return;\n");

    str_add(&host, "    int old_version = current_version;\n");

    str_add(&host, "    current_version = version;\n");

#ifdef _WIN32

    str_add(&host, "    if (hot_lib_handle) FreeLibrary(hot_lib_handle);\n");

    str_printf(&host, "    char dll_path[256];\n");

    str_printf(&host, "    snprintf(dll_path, sizeof(dll_path), \".\\\\%%s_%%d.dll\", \"%s\", version);\n", hot_lib);

    str_add(&host, "    hot_lib_handle = LoadLibraryA(dll_path);\n");

    str_add(&host, "    if (!hot_lib_handle) { fprintf(stderr, \"Failed to load hot library %s_%%d.dll, error code: %%d\\n\", version, (int)GetLastError()); return; }\n");

#else

    str_add(&host, "    if (hot_lib_handle) dlclose(hot_lib_handle);\n");

    str_printf(&host, "    char so_path[256];\n");

    str_printf(&host, "    snprintf(so_path, sizeof(so_path), \"./%s_%%d.so\", version);\n", hot_lib);

    str_add(&host, "    hot_lib_handle = dlopen(so_path, RTLD_LAZY);\n");

    str_add(&host, "    if (!hot_lib_handle) { fprintf(stderr, \"Failed to load hot library: %s\\n\", dlerror()); return; }\n");

#endif

    str_add(&host, "    if (old_version > 0) {\n");

    str_add(&host, "        char old_path[256];\n");

#ifdef _WIN32

    str_printf(&host, "        snprintf(old_path, sizeof(old_path), \"%s_%%d.dll\", old_version);\n", hot_lib);

#else

    str_printf(&host, "        snprintf(old_path, sizeof(old_path), \"./%s_%%d.so\", old_version);\n", hot_lib);

#endif

    str_add(&host, "        remove(old_path);\n");

    str_add(&host, "    }\n");

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_FN) {

            if (strcmp(d->name, "main") == 0) continue; // Skip main lookup

#ifdef _WIN32

            str_printf(&host, "    hot_%s = (void*)GetProcAddress(hot_lib_handle, \"%s\");\n", d->name, d->name);

#else

            str_printf(&host, "    hot_%s = dlsym(hot_lib_handle, \"%s\");\n", d->name, d->name);

#endif

        }

    }

    str_add(&host, "}\n\n");



    str_add(&host, "static void reload_check(void) {\n");

    str_add(&host, "    int new_ver = get_version();\n");

    str_add(&host, "    if (new_ver != current_version) load_hot_functions(new_ver);\n");

    str_add(&host, "}\n\n");



    // === HOST: Stubs and Main Driver ===

    for (int i = 0; i < program.count; i++) {

        Decl *d = program.items[i];

        if (d->kind == DC_FN) {

            bool is_main_args = (strcmp(d->name, "main") == 0 && d->params.count == 1 && d->params.items[0].type->kind == TY_FATPTR);

            bool is_main = (strcmp(d->name, "main") == 0);



            if (is_main_args) {

                // Emit full main body with args directly inside Host

                str_add(&host, "\nint32_t main(int argc, char **argv) {\n");

                str_add(&host, "    load_hot_functions(get_version());\n");

                str_printf(&host, "    tc_fat_ptr %s = { .ptr = argv, .len = (size_t)argc };\n", d->params.items[0].name);

                emit_stmt_vec_with_defers(&host, &d->body, &program, 1);

                str_add(&host, "}\n");

            } else if (is_main) {

                // Emit full void main body directly inside Host

                str_add(&host, "\nint32_t main(void) {\n");

                str_add(&host, "    load_hot_functions(get_version());\n");

                emit_stmt_vec_with_defers(&host, &d->body, &program, 1);

                str_add(&host, "    return 0;\n");

                str_add(&host, "}\n");

            } else {

                // Function stub with reload check

                str_add(&host, "\n"); emit_type(&host, d->type, d->name, &program); str_add(&host, "(");

                if (!d->params.count) str_add(&host, "void");

                for (int j = 0; j < d->params.count; j++) { if (j) str_add(&host, ", "); emit_type(&host, d->params.items[j].type, d->params.items[j].name, &program); }

                str_add(&host, ") {\n");

                str_add(&host, "    reload_check();\n");

                str_add(&host, "    ");

                // Check if return type is void by name

                if (!(d->type->kind == TY_NAME && !strcmp(d->type->name, "void"))) str_add(&host, "return ");

                str_printf(&host, "hot_%s(", d->name);

                for (int j = 0; j < d->params.count; j++) { if (j) str_add(&host, ", "); str_add(&host, d->params.items[j].name); }

                str_add(&host, ");\n}\n");

            }

        }

    }



    *hot_c_out = hot.data;

    return host.data;

}

