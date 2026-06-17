/**
 * parser.h - Syntax Analysis (Parsing)
 *
 * The parser converts a token stream into an Abstract Syntax Tree (AST).
 * It performs syntax analysis using recursive descent parsing with lookahead.
 *
 * The parser:
 * - Recognizes the TIG language grammar
 * - Builds AST nodes for all language constructs
 * - Validates basic syntactic correctness
 * - Reports parse errors with source location
 *
 * Note: Semantic analysis (type checking, variable scope) is performed
 * by the checker phase, not the parser.
 *
 * Example:
 *   Input:  Token stream from lexer
 *   Output: Abstract Syntax Tree (DeclVec containing all program declarations)
 */

#ifndef TIGHTC_PARSER_H
#define TIGHTC_PARSER_H

#include "ast.h"
#include "lexer.h"

// ============================================================================
// Parser Main Function
// ============================================================================

/**
 * parse_program: Parse a complete TIG program
 *
 * Parses a token stream into an AST representing the entire program.
 * The program consists of top-level declarations (functions, structs,
 * enums, imports, variables, and inline C blocks).
 *
 * Grammar:
 *   Program → (Declaration)*
 *   Declaration → Use | Struct | Enum | FunctionDecl | VarDecl | ExternFn | InlineC
 *
 * Args:
 *   tokens: Token stream from lexer (must end with TOK_EOF)
 *   source_file: Path to the source file being parsed (for privacy tracking)
 *
 * Returns:
 *   DeclVec containing all top-level declarations in the program.
 *   The last item is always a null entry marking the end.
 *
 * Error handling:
 *   Reports parse errors via tc_error() when syntax is invalid.
 *   Attempts to recover and continue parsing other declarations.
 *   Returns best-effort AST even if some declarations failed to parse.
 *
 * Example:
 *   TokenVec tokens = lex_source(source);
 *   DeclVec program = parse_program(tokens, "main.tc");
 */
DeclVec parse_program(Token *tokens, const char *source_file);

#endif
