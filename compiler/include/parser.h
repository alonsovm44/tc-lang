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

/**
 * parse_program_with_types: Parse a program with pre-registered type names
 *
 * Same as parse_program, but accepts pre-registered struct/enum names
 * from imported files. This enables the two-phase import mechanism:
 * Phase 1: Collect type names from @use directives
 * Phase 2: Parse with all imported types already registered
 *
 * Args:
 *   tokens: Token stream from lexer
 *   source_file: Path to the source file being parsed
 *   pre_structs: Array of struct names from imported files
 *   pre_struct_count: Number of pre-registered struct names
 *   pre_enums: Array of enum names from imported files
 *   pre_enum_count: Number of pre-registered enum names
 *
 * Returns:
 *   DeclVec containing all top-level declarations in the program
 */
DeclVec parse_program_with_types(Token *tokens, const char *source_file,
                                   char **pre_structs, int pre_struct_count,
                                   char **pre_enums, int pre_enum_count);

// ============================================================================
// Type Registry for Two-Phase Import
// ============================================================================

/**
 * TypeRegistry: Registry for collecting type names from imported files
 *
 * Used in the two-phase import mechanism:
 * Phase 1: Collect struct/enum names from @use directives
 * Phase 2: Register them in the parser's symbol tables before parsing
 */
typedef struct {
    char **structs;
    int struct_count;
    int struct_cap;
    char **enums;
    int enum_count;
    int enum_cap;
} TypeRegistry;

/**
 * collect_imported_types: Collect type names from @use directives
 *
 * Scans a file and its @use imports to collect all struct and enum names.
 * This is Phase 1 of the two-phase import mechanism.
 *
 * Args:
 *   tokens: Token stream from lexer
 *   source_file: Path to the source file being scanned
 *   reg: TypeRegistry to populate with collected type names
 */
void collect_imported_types(Token *tokens, const char *source_file, TypeRegistry *reg);

#endif
