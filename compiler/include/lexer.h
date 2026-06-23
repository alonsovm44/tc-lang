/**
 * lexer.h - Lexical Analysis (Tokenization)
 *
 * The lexer converts raw TIG source code into a stream of tokens.
 * It recognizes keywords, identifiers, literals, operators, and other
 * syntactic elements.
 *
 * The lexer also handles:
 * - Macro expansion (simple text substitution)
 * - Two-character operators (=>, ->, ==, <=, etc.)
 * - String and character literal parsing
 * - Line and column tracking for error reporting
 * - Inline C code blocks (preserved as-is)
 *
 * Example:
 *   Input:  "i32 fn add: i32 a, i32 b { ret a + b }"
 *   Output: [TOK_KEYWORD(i32), TOK_KEYWORD(fn), TOK_IDENT(add), 
 *            TOK_SYMBOL(:), TOK_KEYWORD(i32), TOK_IDENT(a), 
 *            TOK_SYMBOL(,), ...]
 */

#ifndef TIGHTC_LEXER_H
#define TIGHTC_LEXER_H

/**
 * TokenKind: Enumeration of token types
 *
 * TOK_EOF:       End of input marker
 * TOK_IDENT:     Identifier (variable/function name)
 * TOK_NUMBER:    Numeric literal (integer or floating point)
 * TOK_STRING:    String literal (including quotes)
 * TOK_SYMBOL:    Operator or delimiter (single or double char)
 * TOK_KEYWORD:   Reserved keyword (if, loop, fn, ret, etc.)
 * TOK_INLINE_C:  Inline C code block
 * TOK_MACRO:     Macro usage or definition
 */
typedef enum {
    TOK_EOF,        // End of input
    TOK_IDENT,      // Identifier: variable names, function names
    TOK_NUMBER,     // Numeric literal: 42, 3.14, 0xFF
    TOK_STRING,     // String literal: "hello"
    TOK_SYMBOL,     // Operators and delimiters: +, -, {, }, :, etc.
    TOK_KEYWORD,    // Keywords: if, loop, fn, ret, etc.
    TOK_INLINE_C,   // Raw C code block
    TOK_MACRO,      // Macro definition or usage
    TOK_SECTION     // Section directive for OSdev
} TokenKind;

/**
 * Token: A single token from the source
*
 * Represents one indivisible unit of the source code (keyword, identifier,
 * literal, operator, etc.) along with its location information.
 *
 * Fields:
 *   kind: Type of token (identifies what the token represents)
 *   text: String content of the token (e.g., "42", "x", "+")
 *   line: Source line number (1-based) for error reporting
 *   col: Source column number (0-based) for error reporting
 */
typedef struct {
    TokenKind kind;
    char *text;
    int line;
    int col;
} Token;

/**
 * TokenVec: Dynamic array of tokens
 *
 * Used to store the entire token stream produced by the lexer.
 *
 * Fields:
 *   items: Array of tokens
 *   count: Number of tokens in use
 *   cap: Total allocated capacity
 */
typedef struct {
    Token *items;
    int count;
    int cap;
} TokenVec;

// ============================================================================
// Lexer Main Function
// ============================================================================

/**
 * lex_source: Tokenize TIG source code
 *
 * Performs lexical analysis on the input source, converting it into
 * a stream of tokens. Handles all TIG language tokens including keywords,
 * operators, literals, and inline C blocks.
 *
 * The returned token stream includes a final TOK_EOF token to mark the
 * end of input.
 *
 * Args:
 *   src: Complete TIG source code as a null-terminated string
 *
 * Returns:
 *   TokenVec containing all tokens from the source, ending with TOK_EOF
 *
 * Error handling:
 *   Exits with die() if memory allocation fails.
 *   Errors in tokenization are reported via tc_error() and compilation
 *   continues with best-effort recovery.
 *
 * Example:
 *   TokenVec tokens = lex_source("i32 fn main { ret 0 }");
 *   // tokens.count = 6, last token is TOK_EOF
 */
TokenVec lex_source(const char *src);

#endif
