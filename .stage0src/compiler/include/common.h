#ifndef TIGHTC_COMMON_H
#define TIGHTC_COMMON_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} Str;

void die(const char *fmt, ...);

typedef struct {
    const char *filename;
    const char *source;
} SrcContext;

void tc_set_source(const char *filename, const char *source);
void tc_error(const char *ecode, int line, int col, int len, const char *fmt, ...);

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrndup(const char *s, size_t n);
char *xstrdup(const char *s);
void str_push(Str *s, char c);
void str_add(Str *s, const char *text);
void str_printf(Str *s, const char *fmt, ...);
bool is_keyword(const char *text);
bool is_type_name(const char *text);

#endif
