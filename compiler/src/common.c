#include "common.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void die(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

void *xmalloc(size_t size) {
    void *ptr = malloc(size ? size : 1);
    if (!ptr) die("out of memory");
    return ptr;
}

void *xrealloc(void *ptr, size_t size) {
    void *next = realloc(ptr, size ? size : 1);
    if (!next) die("out of memory");
    return next;
}

char *xstrndup(const char *s, size_t n) {
    char *out = xmalloc(n + 1);
    memcpy(out, s, n);
    out[n] = 0;
    return out;
}

char *xstrdup(const char *s) {
    return xstrndup(s, strlen(s));
}

void str_push(Str *s, char c) {
    if (s->len + 1 >= s->cap) {
        s->cap = s->cap ? s->cap * 2 : 128;
        s->data = xrealloc(s->data, s->cap);
    }
    s->data[s->len++] = c;
    s->data[s->len] = 0;
}

void str_add(Str *s, const char *text) {
    while (*text) str_push(s, *text++);
}

void str_printf(Str *s, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    va_list copy;
    va_copy(copy, ap);
    int needed = vsnprintf(NULL, 0, fmt, copy);
    va_end(copy);
    if (needed < 0) die("formatting failed");
    size_t old = s->len;
    while (s->len + (size_t)needed + 1 >= s->cap) {
        s->cap = s->cap ? s->cap * 2 : 128;
        s->data = xrealloc(s->data, s->cap);
    }
    vsnprintf(s->data + old, (size_t)needed + 1, fmt, ap);
    va_end(ap);
    s->len += (size_t)needed;
}

bool is_keyword(const char *text) {
    static const char *keywords[] = {
        "if", "loop", "break", "defer", "ret", "struct", "fn", "use", "pub", "pin", "extern",
        "void", "i2", "i4", "i8", "i16", "i32", "i64", "u2", "u4", "u8", "u16", "u32", "u64", "f32", "f64",
        NULL
    };
    for (int i = 0; keywords[i]; i++) {
        if (strcmp(text, keywords[i]) == 0) return true;
    }
    return false;
}

bool is_type_name(const char *text) {
    static const char *types[] = {
        "void", "i2", "i4", "i8", "i16", "i32", "i64", "u2", "u4", "u8", "u16", "u32", "u64", "f32", "f64", NULL
    };
    for (int i = 0; types[i]; i++) {
        if (strcmp(text, types[i]) == 0) return true;
    }
    return false;
}
