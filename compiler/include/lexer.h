#ifndef TIGHTC_LEXER_H
#define TIGHTC_LEXER_H

typedef enum {
    TOK_EOF,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,
    TOK_SYMBOL,
    TOK_KEYWORD,
    TOK_INLINE_C,
    TOK_MACRO
} TokenKind;

typedef struct {
    TokenKind kind;
    char *text;
    int line;
    int col;
} Token;

typedef struct {
    Token *items;
    int count;
    int cap;
} TokenVec;

TokenVec lex_source(const char *src);

#endif
