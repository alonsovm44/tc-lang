#include "lexer.h"

#include "common.h"
#include "ast.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

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

static char *replace_parameter(const char *text, const char *param, const char *arg) {
    // Simple string replacement implementation
    size_t text_len = strlen(text);
    size_t param_len = strlen(param);
    size_t arg_len = strlen(arg);
    
    // Count occurrences
    int count = 0;
    const char *p = text;
    while ((p = strstr(p, param)) != NULL) {
        count++;
        p += param_len;
    }
    
    if (count == 0) {
        return xstrdup(text);
    }
    
    // Allocate result
    size_t result_len = text_len + count * (arg_len - param_len);
    char *result = xmalloc(result_len + 1);
    
    // Replace
    const char *src = text;
    char *dst = result;
    while (*src) {
        if (strncmp(src, param, param_len) == 0) {
            // Check if this is a whole word match (not part of another identifier)
            const char *next = src + param_len;
            bool is_word_boundary = (!isalnum((unsigned char)*next) && *next != '_') || (*next == '\0');
            bool prev_ok = (src == text) || (!isalnum((unsigned char)*(src-1)) && *(src-1) != '_');
            
            if (is_word_boundary && prev_ok) {
                strcpy(dst, arg);
                dst += arg_len;
                src += param_len;
            } else {
                *dst++ = *src++;
            }
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    
    return result;
}

static char *trim_whitespace_copy(const char *str) {
    const char *start = str;
    const char *end = str + strlen(str) - 1;
    
    // Trim leading space
    while (isspace((unsigned char)*start)) start++;
    
    if (*start == 0) return xstrdup("");
    
    // Trim trailing space
    while (end > start && isspace((unsigned char)*end)) end--;
    
    // Return trimmed copy
    return xstrndup(start, (size_t)(end - start + 1));
}

static char *expand_parametric_macro(Macro *macro, const char *args_str) {
    if (!macro->is_parametric || macro->param_count == 0) {
        return xstrdup(macro->replacement);
    }
    
    // Parse arguments
    char **args = xmalloc(sizeof(char*) * (size_t)macro->param_count);
    int arg_count = 0;
    
    const char *arg_start = args_str;
    const char *arg_end = args_str;
    int paren_count = 0;
    
    while (*arg_end && (arg_count < macro->param_count)) {
        if (*arg_end == '(') paren_count++;
        else if (*arg_end == ')') paren_count--;
        else if (*arg_end == ',' && paren_count == 0) {
            // Extract argument
            args[arg_count] = xstrndup(arg_start, (size_t)(arg_end - arg_start));
            arg_count++;
            arg_start = arg_end + 1;
        }
        arg_end++;
    }
    
    // Add last argument
    if (arg_count < macro->param_count && arg_start < arg_end) {
        args[arg_count] = xstrndup(arg_start, (size_t)(arg_end - arg_start));
        arg_count++;
    }
    
    // Simple parameter replacement
    char *result = xstrdup(macro->replacement);
    
    for (int i = 0; i < macro->param_count && i < arg_count; i++) {
        // Replace all occurrences of parameter with argument
        char *old_result = result;
        result = replace_parameter(old_result, macro->params[i], args[i]);
        free(old_result);
    }
    
    // Clean up
    for (int i = 0; i < arg_count; i++) {
        free(args[i]);
    }
    free(args);
    
    return result;
}

static Macro *find_macro(const char *name) {
    for (int i = global_macros.count - 1; i >= 0; i--) {
        if (strcmp(global_macros.items[i]->name, name) == 0) {
            return global_macros.items[i];
        }
    }
    return NULL;
}

static void add_parametric_macro(const char *name, const char *replacement, char **params, int param_count, bool is_parametric) {
    Macro *m = xmalloc(sizeof(Macro));
    m->name = xstrdup(name);
    m->replacement = xstrdup(replacement);
    m->params = params;
    m->param_count = param_count;
    m->is_parametric = is_parametric;
    
    if (global_macros.count == global_macros.cap) {
        global_macros.cap = global_macros.cap ? global_macros.cap * 2 : 16;
        global_macros.items = xrealloc(global_macros.items, sizeof(Macro*) * (size_t)global_macros.cap);
    }
    global_macros.items[global_macros.count++] = m;
}

static TokenVec lex_source_with_depth(const char *src, int depth);

TokenVec lex_source(const char *src) {
    return lex_source_with_depth(src, 0);
}

static TokenVec lex_source_with_depth(const char *src, int depth) {
    // Prevent infinite recursion in macro expansion
    if (depth > 10) {
        TokenVec out = {0};
        token_push(&out, (Token){TOK_EOF, xstrdup(""), 1, 1});
        return out;
    }
    
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
        // Handle macro definitions: # name(params,...) { replacement }
        if (src[i] == '#') {
            int start = i++;
            col++;
            
            // Skip whitespace after #
            while (src[i] && isspace((unsigned char)src[i])) {
                if (src[i] == '\n') { line++; col = 1; } else { col++; }
                i++;
            }
            
            // Extract macro name
            while (src[i] && !isspace((unsigned char)src[i]) && src[i] != '(' && src[i] != '{') {
                if (src[i] == '\n') { line++; col = 1; } else { col++; }
                i++;
            }
            
            // Check if this is a parametric macro
            bool is_parametric = false;
            if (src[i] == '(') {
                is_parametric = true;
                i++; col++;
                
                // Skip parameters (for now, just find the closing ')')
                int paren_count = 1;
                while (src[i] && paren_count > 0) {
                    if (src[i] == '(') paren_count++;
                    else if (src[i] == ')') paren_count--;
                    else if (src[i] == '\n') { line++; col = 1; i++; continue; }
                    i++; col++;
                }
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
                while (*macro_start && !isspace((unsigned char)*macro_start) && *macro_start != '(' && *macro_start != '{') macro_start++;
                char *name_end = macro_start;
                
                // Extract parameters if parametric
                char **params = NULL;
                int param_count = 0;
                bool is_parametric = false;
                
                if (*macro_start == '(') {
                    is_parametric = true;
                    macro_start++; // Skip (
                    
                    // Parse parameters
                    while (*macro_start && *macro_start != ')') {
                        // Skip whitespace
                        while (*macro_start && isspace((unsigned char)*macro_start)) macro_start++;
                        
                        if (*macro_start == ')') break;
                        
                        // Extract parameter name
                        char *param_start = macro_start;
                        while (*macro_start && !isspace((unsigned char)*macro_start) && *macro_start != ',' && *macro_start != ')') macro_start++;
                        char *param_end = macro_start;
                        
                        // Add parameter to list
                        if (param_count == 0) {
                            params = xmalloc(sizeof(char*));
                        } else {
                            params = xrealloc(params, sizeof(char*) * (size_t)(param_count + 1));
                        }
                        params[param_count] = xstrndup(param_start, (size_t)(param_end - param_start));
                        param_count++;
                        
                        // Skip whitespace and comma
                        while (*macro_start && isspace((unsigned char)*macro_start)) macro_start++;
                        if (*macro_start == ',') macro_start++;
                    }
                    
                    if (*macro_start == ')') macro_start++; // Skip )
                }
                
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
                    add_parametric_macro(name, replacement, params, param_count, is_parametric);
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
            Macro *macro = find_macro(text);
            if (macro) {
                char *expanded = NULL;
                
                if (macro->is_parametric && src[i] == '(') {
                    // This is a parametric macro call - parse arguments
                    i++; col++; // Skip '('
                    
                    int arg_start = i;
                    int paren_count = 1;
                    while (src[i] && paren_count > 0) {
                        if (src[i] == '(') paren_count++;
                        else if (src[i] == ')') paren_count--;
                        else if (src[i] == '\n') { line++; col = 1; i++; continue; }
                        i++; col++;
                    }
                    
                    if (paren_count > 0) {
                        tc_error("E012", start_line, start_col, 1, "unterminated macro call");
                    }
                    
                    // Extract arguments string (without outer parentheses)
                    char *args_str = xstrndup(src + arg_start, (size_t)(i - arg_start - 1));
                    
                    // Expand the macro
                    expanded = expand_parametric_macro(macro, args_str);
                    free(args_str);
                } else {
                    // Simple macro expansion
                    expanded = xstrdup(macro->replacement);
                }
                
                // For now, handle simple single-line expansions properly
                // Multi-line macro expansion needs more sophisticated handling
                if (expanded && strlen(expanded) > 0) {
                    // Check if the expanded text contains newlines (multi-line)
                    bool has_newlines = (strchr(expanded, '\n') != NULL);
                    
                    if (!has_newlines) {
                        // Single line expansion - determine proper token type
                        TokenKind kind = TOK_IDENT;
                        
                        // Check if it's a number
                        if (isdigit((unsigned char)expanded[0]) || 
                            (expanded[0] == '-' && isdigit((unsigned char)expanded[1]))) {
                            kind = TOK_NUMBER;
                        }
                        // Check if it's a string
                        else if (expanded[0] == '"' && expanded[strlen(expanded)-1] == '"') {
                            kind = TOK_STRING;
                        }
                        // Check if it's a keyword
                        else if (is_keyword(expanded)) {
                            kind = TOK_KEYWORD;
                        }
                        
                        token_push(&out, (Token){kind, expanded, start_line, start_col});
                    } else {
                        // Multi-line expansion - for now, create a simple workaround
                        // Split by semicolons and create multiple tokens
                        char *line_start = expanded;
                        char *line_end = expanded;
                        
                        while (*line_end) {
                            if (*line_end == '\n') {
                                // Extract line
                                char *line = xstrndup(line_start, (size_t)(line_end - line_start));
                                if (strlen(line) > 0) {
                                    token_push(&out, (Token){TOK_IDENT, line, start_line, start_col});
                                }
                                free(line);
                                line_start = line_end + 1;
                            }
                            line_end++;
                        }
                        
                        // Handle last line
                        if (line_start < line_end) {
                            char *last_line = xstrndup(line_start, (size_t)(line_end - line_start));
                            if (strlen(last_line) > 0) {
                                token_push(&out, (Token){TOK_IDENT, last_line, start_line, start_col});
                            }
                            free(last_line);
                        }
                        
                        free(expanded);
                        expanded = NULL;
                    }
                }
                
                if (expanded) {
                    free(expanded);
                }
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
