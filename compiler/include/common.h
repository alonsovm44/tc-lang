/**
 * common.h - Common Utilities and Definitions
 *
 * This module provides utility functions and common data structures used
 * throughout the compiler:
 *
 * - Memory management: safe allocation/reallocation with error handling
 * - String manipulation: dynamic strings and utilities
 * - Error reporting: colored diagnostics with source context
 * - Helpers: keyword recognition, type name checking
 *
 * All memory allocation functions use xmalloc/xrealloc which abort if
 * allocation fails, so NULL checks are not needed.
 */

#ifndef TIGHTC_COMMON_H
#define TIGHTC_COMMON_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Str: Dynamic string buffer
 *
 * Used for building strings incrementally (e.g., generated C code).
 *
 * Fields:
 *   data: Allocated character buffer
 *   len: Number of characters in use
 *   cap: Total allocated capacity
 */
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} Str;

/**
 * SrcContext: Source file context for error reporting
 *
 * Maintains the current input file and source code being processed,
 * used to display error messages with source line and column info.
 *
 * Fields:
 *   filename: Name of the source file
 *   source: Complete source code text
 */
typedef struct {
    const char *filename;
    const char *source;
} SrcContext;

// ============================================================================
// Memory Management
// ============================================================================

/**
 * xmalloc: Safe memory allocation with error handling
 *
 * Allocates memory and exits the program if allocation fails.
 * The returned pointer is guaranteed to be non-NULL.
 *
 * Args:
 *   size: Number of bytes to allocate
 * Returns:
 *   Pointer to allocated memory, never NULL
 */
void *xmalloc(size_t size);

/**
 * xrealloc: Safe memory reallocation with error handling
 *
 * Reallocates memory and exits the program if reallocation fails.
 * If ptr is NULL, behaves like xmalloc.
 *
 * Args:
 *   ptr: Previous allocation (may be NULL)
 *   size: New size in bytes
 * Returns:
 *   Pointer to reallocated memory, never NULL
 */
void *xrealloc(void *ptr, size_t size);

/**
 * xstrndup: Safe string duplication (limited length)
 *
 * Allocates memory and copies n characters from string.
 * Always null-terminates the result.
 *
 * Args:
 *   s: Source string
 *   n: Maximum characters to copy
 * Returns:
 *   Newly allocated copy, never NULL
 */
char *xstrndup(const char *s, size_t n);

/**
 * xstrdup: Safe string duplication
 *
 * Allocates memory and copies the entire string.
 *
 * Args:
 *   s: Source string
 * Returns:
 *   Newly allocated copy, never NULL
 */
char *xstrdup(const char *s);

// ============================================================================
// String Building
// ============================================================================

/**
 * str_push: Append single character to dynamic string
 * Args:
 *   s: Target string buffer
 *   c: Character to append
 */
void str_push(Str *s, char c);

/**
 * str_add: Append C string to dynamic string
 * Args:
 *   s: Target string buffer
 *   text: Null-terminated string to append
 */
void str_add(Str *s, const char *text);

/**
 * str_printf: Append formatted string (like sprintf) to dynamic string
 * Args:
 *   s: Target string buffer
 *   fmt: Format string (printf-style)
 *   ...: Format arguments
 */
void str_printf(Str *s, const char *fmt, ...);

// ============================================================================
// Error Reporting
// ============================================================================

/**
 * tc_set_source: Set source context for error reporting
 *
 * Must be called before tc_error to provide file and source context.
 *
 * Args:
 *   filename: Name of the input file
 *   source: Complete source code text
 */
void tc_set_source(const char *filename, const char *source);

/**
 * tc_error: Report a compiler error with source context
 *
 * Prints a colored error message with the source line and carets
 * pointing to the error location. Automatically displays source context
 * set with tc_set_source().
 *
 * Args:
 *   ecode: Error code (e.g., "E001")
 *   line: Line number in source (1-based)
 *   col: Column number in source (0-based)
 *   len: Length of error region in characters
 *   fmt: Error message format (printf-style)
 *   ...: Format arguments
 */
void tc_error(const char *ecode, int line, int col, int len, const char *fmt, ...);

/**
 * die: Report a fatal error and exit
 *
 * Prints an error message and terminates the program.
 * Used for unrecoverable errors like memory allocation failure.
 *
 * Args:
 *   fmt: Error message format (printf-style)
 *   ...: Format arguments
 */
void die(const char *fmt, ...);

// ============================================================================
// Keyword and Type Checking
// ============================================================================

/**
 * is_keyword: Check if a string is a TIG reserved keyword
 *
 * Args:
 *   text: String to check
 * Returns:
 *   true if text is a keyword (if, loop, fn, ret, etc.)
 *   false otherwise
 */
bool is_keyword(const char *text);

/**
 * is_type_name: Check if a string is a built-in type name
 *
 * Args:
 *   text: String to check
 * Returns:
 *   true if text is a type name (i32, i64, u8, void, etc.)
 *   false otherwise
 */
bool is_type_name(const char *text);

#endif
