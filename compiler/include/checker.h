/**
 * checker.h - Semantic Analysis and Type Checking
 *
 * The checker performs semantic analysis on the AST:
 *
 * - Type checking: Validates type compatibility in operations
 * - Variable scoping: Ensures variables are declared before use
 * - Function validation: Checks function signatures and calls
 * - Struct/Enum validation: Validates type definitions
 * - Control flow: Validates return statements and control flow
 * - Async operations: Validates async function usage
 * - Ownership: Validates ownership transfer with @ operator
 * - Memory safety: Detects use-after-free and invalid operations
 *
 * The checker walks the AST recursively, building symbol tables and
 * validating semantic rules. It reports errors via tc_error() for
 * violations found.
 *
 * Output: The AST is modified in-place with type information, and
 * errors are reported. If severe errors are found, compilation should stop.
 */

#ifndef TIGHTC_CHECKER_H
#define TIGHTC_CHECKER_H

#include "ast.h"

// ============================================================================
// Checker Main Function
// ============================================================================

/**
 * check_program: Perform semantic analysis on a complete program
 *
 * Validates the entire AST for semantic correctness:
 *   1. Checks all function and type declarations
 *   2. Validates types in expressions
 *   3. Ensures variables are defined before use
 *   4. Validates function calls match their signatures
 *   5. Checks control flow and return statements
 *   6. Validates struct/enum definitions and usage
 *   7. Validates async operations (for 1.3)
 *   8. Checks ownership and resource handling
 *
 * The program AST is modified in-place to fill in type information.
 *
 * Args:
 *   program: Pointer to the DeclVec AST to check. Modified in-place
 *            to add type information.
 *
 * Returns:
 *   void. Errors are reported via tc_error(). Compilation should stop
 *         if severe errors are reported.
 *
 * Error Categories:
 *   - Type errors: Incompatible types in operations
 *   - Scope errors: Undefined variables/functions
 *   - Function errors: Wrong argument types, missing return
 *   - Struct errors: Invalid field access, missing fields
 *   - Async errors: Invalid async usage or ownership issues
 *   - Flow errors: Unreachable code, missing return statement
 *
 * Example:
 *   DeclVec program = parse_program(tokens);
 *   check_program(&program);  // Validates and annotates AST
 *   if (error_count > 0) {
 *       fprintf(stderr, "Semantic errors found\n");
 *       exit(1);
 *   }
 */
void check_program(DeclVec *program);

#endif
