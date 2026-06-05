/**
 * emitter.h - Code Generation (Emit C11 Code)
 *
 * The emitter generates C11 source code from the validated AST.
 *
 * Key responsibilities:
 * - Convert AST nodes to equivalent C code
 * - Generate struct and type definitions in C
 * - Generate function implementations
 * - Include necessary headers and runtime code
 * - Handle type conversions and casts
 * - Generate async runtime code (when needed)
 * - Support hot-swapping for dynamic library loading
 * - Format output for readability
 *
 * The emitter walks the AST and builds a C program that, when compiled
 * with a C compiler, produces the equivalent behavior of the TIG program.
 *
 * Output: Generated C code is returned as a string to be written to
 * a file or piped to a C compiler.
 *
 * Example:
 *   Checked AST from checker phase
 *   ↓
 *   [Emitter]
 *   ↓
 *   Generated C11 code ready for compilation
 */

#ifndef TIGHTC_EMITTER_H
#define TIGHTC_EMITTER_H

#include "ast.h"

// ============================================================================
// Code Generation Functions
// ============================================================================

/**
 * emit_program: Generate C11 code from a validated AST
 *
 * Converts the complete AST into equivalent C11 code. This is the main
 * entry point for code generation.
 *
 * Generation includes:
 *   1. Generated includes and preamble
 *   2. Type definitions (structs, enums, typedefs)
 *   3. Forward declarations for recursive functions
 *   4. Global variable declarations
 *   5. Function implementations
 *   6. Async runtime code (if async functions are used)
 *   7. Main entry point wrapper (if needed)
 *
 * Args:
 *   program: The validated AST (DeclVec) from the checker phase
 *
 * Returns:
 *   A dynamically allocated string containing the generated C code.
 *   The caller is responsible for freeing this string.
 *   Never returns NULL (exits on allocation failure).
 *
 * Code Quality:
 *   - Includes indentation for readability
 *   - Preserves comments where applicable
 *   - Generates valid C11-compliant code
 *   - Includes necessary headers for stdlib/runtime
 *
 * Async Handling:
 *   - Automatically detects if program uses async functions
 *   - Only includes async runtime if needed (zero overhead for sync)
 *   - Generates thread pool and task management code
 *
 * Example:
 *   DeclVec program = parse_program(tokens);
 *   check_program(&program);
 *   char *c_code = emit_program(program);
 *   // Write c_code to file or compile directly
 *   free(c_code);
 */
char *emit_program(DeclVec program);

/**
 * emit_hot_split: Generate C code for hot-swapping (library reloading)
 *
 * Generates two separate C files for hot-swapping support:
 *   - Main C file: Contains core logic and hot reload infrastructure
 *   - Reloadable library: Contains functions marked for hot-loading
 *
 * This allows updating the library at runtime without restarting
 * the main program. Useful for development and some production scenarios.
 *
 * Args:
 *   program: The validated AST
 *   hot_lib: Name/path of the hot-swappable library
 *   hot_c_out: Output pointer for the library-specific C code.
 *              Will be set to a newly allocated string containing
 *              the library code. Caller must free this string.
 *
 * Returns:
 *   A dynamically allocated string containing the main C code.
 *   The caller is responsible for freeing this string.
 *   The library code is returned via hot_c_out pointer.
 *   Never returns NULL (exits on allocation failure).
 *
 * Generated Files:
 *   - Main code: Loader, main entry point, non-reloadable functions
 *   - Library code: Reloadable functions (marked with is_hot)
 *   - Header: Shared declarations between main and library
 *
 * Example:
 *   char *main_c = NULL;
 *   char *lib_c = NULL;
 *   main_c = emit_hot_split(program, "hotlib.so", &lib_c);
 *   // Write main_c and lib_c to separate files
 *   free(main_c);
 *   free(lib_c);
 */
char *emit_hot_split(DeclVec program, const char *hot_lib, char **hot_c_out);

#endif
