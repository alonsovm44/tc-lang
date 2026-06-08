<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: 45602726f9a02f24
timestamp: 2026-06-07T22:27:31Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# Parser Documentation

## Overview
The `Parser` struct is responsible for parsing the input source code. It takes a sequence of tokens as an argument and generates a vector (`DeclVec`) containing all parsed declarations (functions, structs, enums, etc.). The provided functions handle various aspects of the parsing process, including token manipulation, macro definition parsing, and declaration construction.

## Data Structures

### Parser
- **tokens**: A pointer to an array of tokens in lexical order.
- **pos**: Current position within the `tokens` array.
- **decls**: A vector of parsed declarations (`DeclVec`) containing structs, enums, functions, etc. The structure is initialized with a default value and can be modified by function calls.
- **structs**: An allocated string array storing struct names. It grows dynamically to accommodate new entries.
- **enum_count** & **enums**: Track the number of enum declarations and store their corresponding strings respectively.
- **pin_cap** & **pinned**: Dynamic allocation for pinned variable names, allowing up to `pin_cap` elements.
- **macros**: A vector for storing macro definitions.

### DeclVec
A vector that stores `Decl*` pointers. Each declaration (`Decl`) can represent different kinds of declarations (e.g., structs, enums, functions) and is managed through the use of a `decl_push(&self.decls, decl)` function which will either append to the existing list or reallocate memory if necessary.

### Decl
Represents a parsed declaration such as `struct`, `enum`, `fn`, etc. This structure contains fields indicating whether it's public (`public`), packed (`packed`), and its type information (e.g., for `fn`, it stores return and parameter types). It also has vectors of parameters, methods/fields, and arms/match patterns if applicable.

### Token
Base token struct containing text, kind (e.g., TOK_IDENT for an identifier), line, col, etc. Used throughout the parsing process to manipulate tokens directly without exposing internal details like the `Parser` structure itself.

## Functions

### Macros
- **parse_macro(Parser *p)**: Parses a macro definition. The function skips whitespace and looks for keywords like "macro" or `{ replacement }". It then populates a Macro struct with macro name, replacement text, and other fields.
- **find_macro(Parser *p, const char *name)**: Searches the existing macros to find a match for a given macro name.

### Declarations
- **parse_struct(Parser *p, bool public, bool packed)**: Parses `struct` declarations. Adds struct names to an array if not already present and handles field definitions as specified by the parsing logic.
- **parse_enum(Parser *p, bool public)**: Parses `enum` declarations. Tracks enum names and their values in a vector.
- **resolve_path(const char *base, const char *rel, char *out, size_t out_size)**: Helper function for resolving paths relative to a base path.

### Statements & Expressions
- **parse_expr(Parser *p)**, **parse_init_stmts**, **parse_block**, **parse_stmt**: Core functions used in the parsing pipeline to build statements and expressions. These involve checking for special syntax like loops, selects, pins, etc., as well as handling expressions.
- **expect**, **macro_push**, **add_struct**, **add_enum**: Helper functions that handle validation, dynamic memory management, and tracking of various elements (like macros or struct declarations).

### General Parsing
- **match(Parser *p, const char *text)**: Checks if the current token matches a given text.
- **at(Parser *p, const char *text)**: Compares current token against a given string to check for exact match.

## Notes

This parser is designed with extensibility in mind, including support for macros and various new syntax features like async `fn` declarations. Error handling is integrated throughout the functions, providing helpful error messages upon encountering unexpected input or malformed expressions/declarations.

The code structure supports dynamic memory allocation to handle a potentially large number of declarations and macro definitions efficiently, ensuring that allocations grow as needed rather than being fixed sizes which can cause performance issues with very complex source files.