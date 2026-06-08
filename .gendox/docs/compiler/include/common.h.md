<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: bd44f7116cad330f
timestamp: 2026-06-07T22:01:23Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# TIGHTC Common Utilities and Definitions

This module provides utility functions and common data structures used throughout the compiler. The main focus areas include memory management, string manipulation, error reporting, and helpers for keyword and type checking.

## Str: Dynamic String Buffer

### Purpose:
Used for building strings incrementally (e.g., generated C code).

### Fields:
- `data`: Allocated character buffer
- `len`: Number of characters in use
- `cap`: Total allocated capacity

```markdown
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} Str;
```

## SrcContext: Source File Context for Error Reporting

### Purpose:
Maintains the current input file and source code being processed, used to display error messages with source line and column info.

### Fields:
- `filename`: Name of the source file
- `source`: Complete source code text

```markdown
typedef struct {
    const char *filename;
    const char *source;
} SrcContext;
```

## Memory Management

### xmalloc: Safe Memory Allocation with Error Handling

Allocates memory and exits the program if allocation fails. The returned pointer is guaranteed to be non-NULL.

```markdown
void *xmalloc(size_t size);
```

### xrealloc: Safe Memory Reallocation with Error Handling

Reallocates memory and exits the program if reallocation fails. If ptr is NULL, behaves like `xmalloc`.

```markdown
void *xrealloc(void *ptr, size_t size);
```

### xstrndup: Safe String Duplication (Limited Length)

Allocates memory and copies n characters from string. Always null-terminates the result.

```markdown
char *xstrndup(const char *s, size_t n);
```

### xstrdup: Safe String Duplication

Allocates memory and copies the entire string.

```markdown
char *xstrdup(const char *s);
```

## String Building

### str_push: Append Single Character to Dynamic String

Appends a single character to the dynamic string buffer.

```markdown
void str_push(Str *s, char c);
```

### str_add: Append C String to Dynamic String

Appends a null-terminated string (C string) to the dynamic string buffer.

```markdown
void str_add(Str *s, const char *text);
```

### str_printf: Append Formatted String (Like `sprintf`)

Appends a formatted string to the dynamic string buffer. This function allows for formatting via printf-style arguments.

```markdown
void str_printf(Str *s, const char *fmt, ...);
```

## Error Reporting

### tc_set_source: Set Source Context for Error Reporting

Must be called before `tc_error` to provide file and source context. Sets the current input file and source code text.

```markdown
void tc_set_source(const char *filename, const char *source);
```

### tc_error: Report a Compiler Error with Source Context

Reports a compiler error with source context. Automatically displays source context set by `tc_set_source()`. Displays an error message along with the line number and column numbers where the error occurred.

```markdown
void tc_error(const char *ecode, int line, int col, int len, const char *fmt, ...);
```

### die: Report a Fatal Error and Exit

Reports a fatal error and terminates the program. Used for unrecoverable errors like memory allocation failure.

```markdown
void die(const char *fmt, ...);
```

## Keyword and Type Checking

### is_keyword: Check if a String is a TIG Reserved Keyword

Checks if the provided string is a reserved keyword in the compiler's set of keywords (e.g., `if`, `loop`, `fn`, `ret`).

```markdown
bool is_keyword(const char *text);
```

### is_type_name: Check if a String is a Built-in Type Name

Checks if the provided string represents a built-in type name (e.g., `i32`, `u8`, `void`).

```markdown
bool is_type_name(const char *text);
```