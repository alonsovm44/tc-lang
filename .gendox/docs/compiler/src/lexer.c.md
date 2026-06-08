<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: 4cbcd9fe36af5f2c
timestamp: 2026-06-07T22:21:34Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# Lexer Implementation Documentation

## Overview

The lexer in this code is designed to process a source code file and convert it into an abstract syntax tree (AST) of tokens. The function `lex_source` takes the raw source code as input and outputs a `TokenVec`, which is a structure containing lists of parsed tokens, including keywords, symbols, numbers, strings, identifiers, and error messages.

### Key Components

- **Token**: A structure that describes a token with its type (like TOK_EOF for end-of-file), value (the string content or identifier), line number, and column number.
  
- **MacroVec**: A vector to store macros defined by the user. It stores pointers to `Macro` structures.

- **Macro**: Defines a macro definition including its name (`name`), replacement text (`replacement`), parameters list (`params`), parameter count (`param_count`), and whether it is parametric (`is_parametric`).

## Functions

### add_macro()

```markdown
#### Purpose:
- Adds a new macro to the global macros collection.

#### Parameters:
- `const char *name`: The name of the macro.
- `const char *replacement`: The replacement text for the macro.

#### Return:
- None. 

#### Usage:
Call this function when you define a macro in your source code.

#### Example:

```c
add_macro("macro_name", "replacement_text");
```
```

### expand_parametric_macro()

```markdown
#### Purpose:
- Expands parametric macros with corresponding arguments, handling cases where the macro definition includes parameter placeholders.

#### Parameters:
- `Macro *macro`: A pointer to the macro structure.
- `const char *args_str`: The string containing the arguments (enclosed in parentheses) that will be substituted into the macro's replacement text.

#### Return:
- Returns a new token vector with the expanded result of the macro, or NULL if an error occurs.

#### Usage:
Call this function when you need to expand parametric macros within your source code.

#### Example:

```c
char *expanded_macro = expand_parametric_macro(macro_ptr, params_str);
```

### find_macro()

```markdown
#### Purpose:
- Searches for a macro by its name in the global collection of macros.

#### Parameters:
- `const char *name`: The name of the macro to search for.

#### Return:
- Returns a pointer to the found macro structure or NULL if not found.

#### Usage:
Use this function when you need to access information about a particular macro during parsing.

#### Example:

```c
Macro *macro = find_macro("macro_name");
```

### add_parametric_macro()

```markdown
#### Purpose:
- Adds parametric macros (macros that take parameters) into the global collection of macros.

#### Parameters:
- `const char *name`: The name of the macro.
- `const char *replacement`: The replacement text for the macro.
- `char **params`: A pointer to an array containing parameter strings.
- `int param_count`: Number of parameters in the macro.
- `bool is_parametric`: Whether the macro is a parametric one.

#### Return:
- None. 

#### Usage:
Use this function when defining new parametric macros within your codebase.

#### Example:

```c
add_parametric_macro("parametric_macro", "replacement_text", params, 2, true);
```

### lex_source()

```markdown
#### Purpose:
- Lexes a source string and returns the corresponding token vector. It is used internally by more specific lexer implementations like `lex_source_with_depth`.

#### Parameters:
- `const char *src`: The raw source code to be parsed.

#### Return:
- Returns a `TokenVec` containing all the tokens found in the source.

#### Usage:
Use this function as a starting point for lexing your source code. It will return all types of tokens including numbers, strings, keywords, identifiers, and errors.

#### Example:

```c
TokenVec token_vector = lex_source(source_code);
```

### two_char_symbol()

```markdown
#### Purpose:
- Checks if the given string is a two-character symbol.

#### Parameters:
- `const char *s`: The string to check.

#### Return:
- Returns true if the string matches any of the predefined two-character symbols, false otherwise.

#### Usage:
Use this function when you need to recognize specific two-character symbols in your lexer for certain parsing rules.

#### Example:

```c
if (two_char_symbol(">>")) {
    // Handle the >> symbol case.
}
```

### token_push()

```markdown
#### Purpose:
- Appends a new token to the existing `TokenVec` structure.

#### Parameters:
- `TokenVec *v`: The vector where the token will be appended.
- `Token t`: The new token to add.

#### Return:
- None. 

#### Usage:
Call this function whenever you need to push a new token into the AST representation of your source code.

#### Example:

```c
token_push(&out, (Token){TOK_IDENT, "identifier", 1, 2});
```

### trim_whitespace_copy()

```markdown
#### Purpose:
- Trims leading and trailing whitespace from the input string.

#### Parameters:
- `const char *str`: The source code string to be trimmed.

#### Return:
- Returns a new token vector containing the trimmed content of the original string.

#### Usage:
Use this function when you need to handle strings with potential leading or trailing whitespace in your lexer implementation.

#### Example:

```c
char *trimmed = trim_whitespace_copy(source);
```

### tc_error()

```markdown
#### Purpose:
- Generates an error message based on a specified type and location within the source code, indicating what went wrong during lexing. 

#### Parameters:
- `const char *type`: The error type identifier.
- `int line`: Line number in the source file where the error occurred.
- `int col`: Column number in the source file where the error occurred.
- `size_t size`: Size of the token or source causing the error.

#### Return:
- None. 

#### Usage:
Call this function when you want to signal an error condition during lexing, providing details about where and what kind of problem has arisen.

#### Example:

```c
tc_error("E012", 5, 7, strlen("error_message"), "unterminated macro definition");
```

### is_keyword()

```markdown
#### Purpose:
- Checks if the given string represents a keyword used in programming languages.

#### Parameters:
- `const char *text`: The text to check.

#### Return:
- Returns true if the provided text matches any predefined keywords, false otherwise.

#### Usage:
Utilize this function when you need to determine whether a particular piece of code is recognized as a keyword by your lexer implementation.

#### Example:

```c
if (is_keyword("for")) {
    // Handle "for" keyword.
}
```

## Summary

This lexer code allows for the lexing and parsing of source code into an AST. It supports macros, handling multi-line macros with parameters, error checking and message generation, and proper tokenization including identifiers, keywords, numbers, strings, symbols, and errors. Each function serves a specific purpose in the overall process, ensuring thorough processing and correct interpretation of the input text as per programming language rules.