<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: 9e609b4451fe943b
timestamp: 2026-06-07T22:05:48Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# Lexer Documentation

## Overview

The `lexer.h` module in the TIG (Tiny Intermediate Grammar) project handles lexical analysis for converting raw source code into a stream of meaningful tokens. The lexer recognizes keywords, identifiers, literals, operators, and other syntactic elements from the input string. It also processes two-character operators, macros, strings, character literals, and inline C blocks.

This documentation details the purpose, usage, and behavior of each key component involved in lexical analysis within TIG's compiler.

## Lexer Data Structures

### TokenKind Enumeration

```markdown
**TokenKind: Enumeration of token types**

The `TOK_EOF`, `TOK_IDENT`, `TOK_NUMBER`, `TOK_STRING`, `TOK_SYMBOL`, `TOK_KEYWORD`, and `TOK_INLINE_C` enumerations represent different types of tokens recognized by the lexer. Macros are represented by `TOK_MACRO`.

**Values:**
- TOK_EOF: End of input marker.
- TOK_IDENT: Identifier (variable/function name).
- TOK_NUMBER: Numeric literal (integer or floating point).
- TOK_STRING: String literal (including quotes).
- TOK_SYMBOL: Operator or delimiter (single or double char).
- TOK_KEYWORD: Reserved keyword (if, loop, fn, ret, etc.).
- TOK_INLINE_C: Inline C code block.
- TOK_MACRO: Macro definition or usage.
```

### Token Structure

```markdown
**Token: A single token from the source**

The `struct Token` represents a single unit of code and includes information about its type (`kind`), value (`text`), location in the source (`line`, `col`).

**Fields:**
- kind: Identifies what the token represents (e.g., keyword, identifier, number, string, etc.).
- text: The actual content of the token.
- line: The line number where the token appears, using a 1-based system.
- col: The column within that line, starting at 0.

**Example Usage:**
```c
Token tok = {TOK_NUMBER, "42", 5, 7};
```
In this example:
- `tok.kind` would be set to TOK_NUMBER.
- `tok.text` is the string representation of the value being tokenized (e.g., `"42"`, `"hello"`).
- `tok.line` represents the line number in the source code where the token appears, for instance, 5.
- `tok.col` denotes the column within that line starting from 0, like 7.

### TokenVec Structure

```markdown
**TokenVec: Dynamic array of tokens**

This structure holds a dynamically allocated sequence of `Token` structures. It's used to store all generated tokens and supports efficient appending of new elements.

**Fields:**
- items: A pointer to the array of `Token`.
- count: Number of tokens currently in use.
- cap: Total allocated capacity for the vector, which can be expanded as needed.

**Example Usage:**
```c
// Initialization
TokenVec vec = {NULL, 0, 10};

// Allocating memory and appending a new token
vec.items = (Token*)malloc(vec.cap * sizeof(Token));
vec.count++;
memcpy(&vec.items[vec.count - 1], &tok, sizeof(Token));

// Checking the total count of items now.
```
## Lexer Function

### `lex_source` Function

```markdown
**lex_source: Tokenize TIG source code**

This function performs lexical analysis on the provided input string (`src`) and generates a stream of tokens. The returned structure is of type `TokenVec`, containing all identified tokens along with an additional TOK_EOF token to mark the end of input.

**Args:**
- **`const char * src`:** A null-terminated string representing complete TIG source code that needs to be tokenized.

**Returns:**
- A `TokenVec` structure containing a sequence of tokens (excluding the final TOK_EOF marker) and with `count` set to the total number of processed tokens, including the TOK_EOF marker at the end.
  
**Error Handling:**
- If memory allocation fails during initialization or expansion, the function calls `die()` to terminate execution.
- Errors are reported via `tc_error()`, allowing compilation to continue with best-effort recovery mechanisms.

**Example Usage:**
```c
TokenVec tokens = lex_source("i32 fn main { ret 0 }");
// 'tokens.count' should be 6, with the last token being TOK_EOF.
```

In this example:
- The function `lex_source` is called with a string representing TIG source code (`"i32 fn main { ret 0 }"`).
- It returns a `TokenVec` containing all identified tokens from the input, including an extra `TOK_EOF` at the end.
```

This comprehensive documentation provides insight into how to utilize these lexer components within the TIG compiler project.