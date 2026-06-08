<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: 0e866b28b159ac58
timestamp: 2026-06-07T22:06:39Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# Parser.h - Syntax Analysis (Parsing)

The `parser.h` file contains the definition of a parser for TIG source code, which converts a token stream into an Abstract Syntax Tree (AST). This process involves recognizing TIG language grammar and building AST nodes for all program constructs. The parser ensures basic syntactic correctness but does not perform semantic analysis such as type checking or variable scope validation.

## Parser Overview

The parser is based on recursive descent parsing with lookahead, which means it analyzes the input from left to right while keeping a look ahead of one token. It recognizes various syntax elements like keywords, identifiers, literals, operators, and constructs specific to TIG language (functions, structs, enums, imports, variables, etc.). The goal is to build an Abstract Syntax Tree (AST) that represents all top-level declarations in the program.

## Function: parse_program

### Definition
```c
DeclVec parse_program(Token *tokens);
```

#### Parameters:
- `tokens`: A token stream from the lexer. This parameter should be a pointer to a Token structure array (`Token*`) representing the TIG source code after lexing. The last item in this array must be a TOK_EOF marker indicating the end of the input.

### Returns
- `DeclVec`: A DeclVec containing all top-level declarations in the program as AST nodes. This vector always ends with a null entry marking its end, ensuring consistency in the AST structure.

#### Behavior:
- The parser adheres to TIG language grammar rules and constructs an Abstract Syntax Tree (AST) for valid syntax. It handles multiple declaration types including functions, structs, enums, imports, variables, and inline C code blocks.
  
### Error Handling
- When encountering invalid syntax, the `parse_program` function reports parse errors via `tc_error()`, allowing compilation to continue with best-effort recovery. The parser attempts to recover from erroneous parts of the program while continuing to process valid declarations.

#### Example Usage:
```c
#include "parser.h"
#include "lexer.h"

int main(void) {
    const char *source = "...";  // Assume this contains TIG source code
    TokenVec tokens = lex_source(source);
    DeclVec program = parse_program(tokens.tokens);  // `tokens.tokens` is the array of pointers to each token in the `TokenVec`
    
    if (program.empty()) {
        tc_error("Parsing error: no valid declarations found.");
        return -1;
    }
    
    // Further processing or use of the AST
    ...
}
```

In summary, the `parse_program` function takes a token stream produced by the lexer and constructs an Abstract Syntax Tree representing the TIG program. It is designed to handle various syntax elements efficiently while providing error recovery for invalid input.