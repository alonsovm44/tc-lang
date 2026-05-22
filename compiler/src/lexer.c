#include "lexer.h"

#include "common.h"

#include <ctype.h>
#include <string.h>

static void token_push(TokenVec *v, Token t) {
    if (v->count == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 256;
        v->items = xrealloc(v->items, sizeof(Token) * (size_t)v->cap);
    }
    v->items[v->count++] = t;
}

static bool two_char_symbol(const char *s) {
    static const char *symbols[] = {"->", "=>", "==", "<>", "<=", ">=", "<<", ">>", "&&", "||", "++", "--", "+=", "-=", "*=", "/=", "%=", NULL};
    for (int i = 0; symbols[i]; i++) {
        if (strncmp(s, symbols[i], 2) == 0) return true;
    }
    return false;
}

TokenVec lex_source(const char *src) {
    TokenVec out = {0};
    int i = 0, line = 1, col = 1;
    while (src[i]) {
        char c = src[i];
        if (isspace((unsigned char)c)) {
            if (c == '\n') { line++; col = 1; } else { col++; }
            i++;
            continue;
        }
        if (src[i] == '/' && src[i + 1] == '/') {
            while (src[i] && src[i] != '\n') { i++; col++; }
            continue;
        }
        if (src[i] == '/' && src[i + 1] == '*') {
            i += 2; col += 2;
            while (src[i] && !(src[i] == '*' && src[i + 1] == '/')) {
                if (src[i] == '\n') { line++; col = 1; i++; } else { i++; col++; }
            }
            if (!src[i]) die("unterminated block comment");
            i += 2; col += 2;
            continue;
        }
        int start_line = line, start_col = col;
        if (src[i] == '.' && src[i + 1] == '.' && src[i + 2] == '.') {
            token_push(&out, (Token){TOK_SYMBOL, xstrdup("..."), start_line, start_col});
            i += 3; col += 3;
            continue;
        }
        if (two_char_symbol(src + i)) {
            token_push(&out, (Token){TOK_SYMBOL, xstrndup(src + i, 2), start_line, start_col});
            i += 2; col += 2;
            continue;
        }
        if (strchr("{}()[],:.=+-*/%<>!&|^@", src[i])) {
            token_push(&out, (Token){TOK_SYMBOL, xstrndup(src + i, 1), start_line, start_col});
            i++; col++;
            continue;
        }
        if (src[i] == '"') {
            int start = i++;
            col++;
            while (src[i] && src[i] != '"') {
                if (src[i] == '\\' && src[i + 1]) { i++; col++; }
                if (src[i] == '\n') die("unterminated string at %d:%d", start_line, start_col);
                i++; col++;
            }
            if (!src[i]) die("unterminated string at %d:%d", start_line, start_col);
            i++; col++;
            token_push(&out, (Token){TOK_STRING, xstrndup(src + start, (size_t)(i - start)), start_line, start_col});
            continue;
        }
        if (isdigit((unsigned char)src[i])) {
            int start = i;
            while (isalnum((unsigned char)src[i]) || src[i] == '.') { i++; col++; }
            token_push(&out, (Token){TOK_NUMBER, xstrndup(src + start, (size_t)(i - start)), start_line, start_col});
            continue;
        }
        if (isalpha((unsigned char)src[i]) || src[i] == '_') {
            int start = i;
            while (isalnum((unsigned char)src[i]) || src[i] == '_') { i++; col++; }
            char *text = xstrndup(src + start, (size_t)(i - start));
            token_push(&out, (Token){is_keyword(text) ? TOK_KEYWORD : TOK_IDENT, text, start_line, start_col});
            continue;
        }
        die("unexpected character '%c' at %d:%d", src[i], line, col);
    }
    token_push(&out, (Token){TOK_EOF, xstrdup(""), line, col});
    return out;
}
