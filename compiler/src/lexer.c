#include "lexer.h"

#include "common.h"
#include "ast.h"

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

static MacroVec global_macros = {0};

static void add_macro(const char *name, const char *replacement) {
    Macro *m = xmalloc(sizeof(Macro));
    m->name = xstrdup(name);
    m->replacement = xstrdup(replacement);
    m->params = NULL;
    m->param_count = 0;
    m->is_parametric = false;
    
    if (global_macros.count == global_macros.cap) {
        global_macros.cap = global_macros.cap ? global_macros.cap * 2 : 16;
        global_macros.items = xrealloc(global_macros.items, sizeof(Macro*) * (size_t)global_macros.cap);
    }
    global_macros.items[global_macros.count++] = m;
}

static const char *find_macro_replacement(const char *name) {
    for (int i = global_macros.count - 1; i >= 0; i--) {
        if (strcmp(global_macros.items[i]->name, name) == 0) {
            return global_macros.items[i]->replacement;
        }
    }
    return NULL;
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
            if (!src[i]) tc_error("E012", line, col, 2, "unterminated block comment");
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
        // Handle macro definitions: # name { replacement }
        if (src[i] == '#') {
            int start = i++;
            col++;
            
            // Skip whitespace after #
            while (src[i] && isspace((unsigned char)src[i])) {
                if (src[i] == '\n') { line++; col = 1; } else { col++; }
                i++;
            }
            
            // Extract macro name
            while (src[i] && !isspace((unsigned char)src[i]) && src[i] != '{') {
                if (src[i] == '\n') { line++; col = 1; } else { col++; }
                i++;
            }
            
            // Skip whitespace before {
            while (src[i] && isspace((unsigned char)src[i])) {
                if (src[i] == '\n') { line++; col = 1; } else { col++; }
                i++;
            }
            
            if (src[i] == '{') {
                i++; col++;
                
                // Find matching closing brace
                int brace_count = 1;
                int macro_start_line = line, macro_start_col = col;
                
                while (src[i] && brace_count > 0) {
                    if (src[i] == '{') brace_count++;
                    else if (src[i] == '}') brace_count--;
                    else if (src[i] == '\n') { line++; col = 1; i++; continue; }
                    else if (src[i] == '/' && src[i + 1] == '/') {
                        while (src[i] && src[i] != '\n') { i++; col++; }
                        continue;
                    }
                    else if (src[i] == '/' && src[i + 1] == '*') {
                        i += 2; col += 2;
                        while (src[i] && !(src[i] == '*' && src[i + 1] == '/')) {
                            if (src[i] == '\n') { line++; col = 1; i++; } else { i++; col++; }
                        }
                        if (src[i]) { i += 2; col += 2; }
                        continue;
                    }
                    
                    i++; col++;
                }
                
                if (brace_count > 0) {
                    tc_error("E012", macro_start_line, macro_start_col, 1, "unterminated macro definition");
                }
                
                // Parse and add macro to global list
                const char *macro_start = src + start + 1; // Skip #
                
                // Skip whitespace after #
                while (*macro_start && isspace((unsigned char)*macro_start)) macro_start++;
                
                // Extract macro name
                char *name_start = macro_start;
                while (*macro_start && !isspace((unsigned char)*macro_start) && *macro_start != '{') macro_start++;
                char *name_end = macro_start;
                
                // Skip whitespace before {
                while (*macro_start && isspace((unsigned char)*macro_start)) macro_start++;
                
                if (*macro_start == '{') {
                    macro_start++; // Skip {
                    
                    // Find matching closing brace
                    char *replacement_start = macro_start;
                    char *replacement_end = replacement_start;
                    int brace_count = 1;
                    
                    while (*replacement_end && brace_count > 0) {
                        if (*replacement_end == '{') brace_count++;
                        else if (*replacement_end == '}') brace_count--;
                        replacement_end++;
                    }
                    
                    // Add macro to global list
                    char *name = xstrndup(name_start, (size_t)(name_end - name_start));
                    char *replacement = xstrndup(replacement_start, (size_t)(replacement_end - replacement_start - 1));
                    add_macro(name, replacement);
                }
                
                continue;
            } else {
                // If no { after #, treat # as a symbol
                token_push(&out, (Token){TOK_SYMBOL, xstrndup(src + start, 1), start_line, start_col});
                continue;
            }
        }
        
        // Handle inline C code: "C"{ ... } - must come before string literal check
        if (src[i] == '"' && src[i + 1] == 'C' && src[i + 2] == '"' && src[i + 3] == '{') {
            int start = i;
            i += 4; col += 4;
            
            // Find the matching closing brace
            int brace_count = 1;
            int c_start_line = line, c_start_col = col;
            
            while (src[i] && brace_count > 0) {
                if (src[i] == '{') brace_count++;
                else if (src[i] == '}') brace_count--;
                else if (src[i] == '\n') { line++; col = 1; i++; continue; }
                else if (src[i] == '/' && src[i + 1] == '/') {
                    while (src[i] && src[i] != '\n') { i++; col++; }
                    continue;
                }
                else if (src[i] == '/' && src[i + 1] == '*') {
                    i += 2; col += 2;
                    while (src[i] && !(src[i] == '*' && src[i + 1] == '/')) {
                        if (src[i] == '\n') { line++; col = 1; i++; } else { i++; col++; }
                    }
                    if (src[i]) { i += 2; col += 2; }
                    continue;
                }
                
                i++; col++;
            }
            
            if (brace_count > 0) {
                tc_error("E012", c_start_line, c_start_col, 1, "unterminated inline C block");
            }
            
            // Extract the C code (inside the braces)
            char *c_code = xstrndup(src + start + 4, (size_t)(i - start - 5));
            token_push(&out, (Token){TOK_INLINE_C, c_code, start_line, start_col});
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
                if (src[i] == '\n') tc_error("E012", start_line, start_col, 1, "unterminated string literal");
                i++; col++;
            }
            if (!src[i]) tc_error("E012", start_line, start_col, 1, "unterminated string literal");
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
            
            // Check if this identifier is a macro
            const char *replacement = find_macro_replacement(text);
            if (replacement) {
                // Replace with macro expansion - for now, treat as identifier
                // In a full implementation, we'd need to re-lex the replacement text
                token_push(&out, (Token){TOK_IDENT, xstrdup(replacement), start_line, start_col});
            } else {
                token_push(&out, (Token){is_keyword(text) ? TOK_KEYWORD : TOK_IDENT, text, start_line, start_col});
            }
            continue;
        }
        tc_error("E013", line, col, 1, "unexpected character '%c'", src[i]);
    }
    token_push(&out, (Token){TOK_EOF, xstrdup(""), line, col});
    return out;
}
