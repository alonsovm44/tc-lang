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

static SrcContext g_src = {0};

void tc_set_source(const char *filename, const char *source) {
    g_src.filename = filename;
    g_src.source = source;
}

static const char *get_line_start(const char *src, int target_line) {
    int line = 1;
    const char *p = src;
    while (*p && line < target_line) {
        if (*p == '\n') line++;
        p++;
    }
    return p;
}

static int get_line_len(const char *line_start) {
    int len = 0;
    while (line_start[len] && line_start[len] != '\n') len++;
    return len;
}

// Error collection for multiple error reporting
typedef struct {
    char *ecode;
    int line;
    int col;
    int span;
    char *message;
    char *filename;
    char *source;
} CollectedError;

static CollectedError *g_errors = NULL;
static int g_error_count = 0;
static int g_error_cap = 0;

void tc_error(const char *ecode, int line, int col, int span, const char *fmt, ...) {
    // Collect error instead of exiting immediately
    if (g_error_count == g_error_cap) {
        g_error_cap = g_error_cap ? g_error_cap * 2 : 16;
        g_errors = xrealloc(g_errors, sizeof(CollectedError) * (size_t)g_error_cap);
    }
    
    CollectedError *err = &g_errors[g_error_count++];
    err->ecode = xstrdup(ecode);
    err->line = line;
    err->col = col;
    err->span = span;
    err->filename = g_src.filename ? xstrdup(g_src.filename) : NULL;
    err->source = g_src.source ? xstrdup(g_src.source) : NULL;
    
    // Format the error message
    va_list ap;
    va_start(ap, fmt);
    char msg_buf[1024];
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);
    va_end(ap);
    err->message = xstrdup(msg_buf);
}

void tc_report_errors(void) {
    if (g_error_count == 0) return;
    
    const char *RED    = "\033[1;31m";
    const char *BLUE   = "\033[1;34m";
    const char *CYAN   = "\033[1;36m";
    const char *BOLD   = "\033[1m";
    const char *DIM    = "\033[2m";
    const char *RESET  = "\033[0m";
    
    // Print summary
    fprintf(stderr, "%s", BOLD);
    if (g_error_count == 1) {
        fprintf(stderr, "error: could not compile file");
    } else {
        fprintf(stderr, "error: could not compile file (%d errors)", g_error_count);
    }
    fprintf(stderr, "%s\n\n", RESET);
    
    // Print each error
    for (int i = 0; i < g_error_count; i++) {
        CollectedError *err = &g_errors[i];
        
        fprintf(stderr, "%s%serror[%s]%s%s: ", RED, BOLD, err->ecode, RESET, BOLD);
        fprintf(stderr, "%s%s\n", err->message, RESET);
        
        if (err->filename) {
            fprintf(stderr, " %s-->%s %s:%d:%d\n", BLUE, RESET, err->filename, err->line, err->col);
        }
        
        if (err->source && err->line > 0) {
            const char *ls = get_line_start(err->source, err->line);
            int ll = get_line_len(ls);
            char line_num[16];
            int nw = snprintf(line_num, sizeof(line_num), "%d", err->line);
            
            fprintf(stderr, " %*s %s|%s\n", nw, "", BLUE, RESET);
            fprintf(stderr, " %s%d%s %s|%s %.*s\n", BLUE, err->line, RESET, BLUE, RESET, ll, ls);
            fprintf(stderr, " %*s %s|%s ", nw, "", BLUE, RESET);
            
            int span = err->span;
            if (span < 1) span = 1;
            for (int j = 1; j < err->col; j++) fputc(' ', stderr);
            fprintf(stderr, "%s", RED);
            for (int j = 0; j < span; j++) fputc('^', stderr);
            fprintf(stderr, "%s", RESET);
            
            fprintf(stderr, " %s%s%s\n", RED, err->message, RESET);
        }
        
        fprintf(stderr, "\n%s%s%s\n", CYAN, err->ecode, RESET);
        fprintf(stderr, "%sType \"tcc --error %s\" for help%s\n\n", DIM, err->ecode, RESET);
    }
    
    exit(1);
}

bool tc_has_errors(void) {
    return g_error_count > 0;
}

void tc_clear_errors(void) {
    for (int i = 0; i < g_error_count; i++) {
        free(g_errors[i].ecode);
        free(g_errors[i].message);
        free(g_errors[i].filename);
        free(g_errors[i].source);
    }
    g_error_count = 0;
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
        "if", "else", "loop", "break", "defer", "ret", "struct", "enum", "fn", "use", "pin", "extern", "sizeof",
        "async", "queue", "stack", "select",
        "error", "throw", "try", "catch",
        "raw", "interrupt",
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
        "void", "i2", "i4", "i8", "i16", "i32", "i64", "u2", "u4", "u8", "u16", "u32", "u64", "f32", "f64",
        "queue", "stack", NULL
    };
    for (int i = 0; types[i]; i++) {
        if (strcmp(text, types[i]) == 0) return true;
    }
    return false;
}
