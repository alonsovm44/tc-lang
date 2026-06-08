<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: f2ed53481fde4f9c
timestamp: 2026-06-07T22:02:25Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# Emitter Documentation

## Overview
The `emitter.h` header file contains functions for generating C11 code from a validated Abstract Syntax Tree (AST). The emitter is responsible for converting the AST into equivalent C code, including necessary include files and runtime code. It also handles type definitions, function implementations, async runtime support, and hot-swapping capabilities.

## Functions

### `char *emit_program(DeclVec program)`

**Purpose:**
Generates C11 source code from a validated AST and returns it as a dynamically allocated string that the caller can write to a file or pipe to a compiler. The generated code is designed to mimic the behavior of the TIG (TightC Compiler) program.

**Arguments:**
- `program`: A DeclVec representing the validated AST from the checker phase.

**Return Value:**
A dynamically allocated string containing the generated C11 code. The caller must free this string after use with `free(c_code)` to avoid memory leaks.

**Code Quality Requirements:**
The output includes:
- Indentation for readability
- Preserved comments where applicable
- Valid C11-compliant code
- Necessary headers like `<stdlib.h>` and others included as required by the generated code

**Async Handling:**
The function automatically detects if the program uses async functions. If so, it generates thread pool and task management code to handle asynchronous operations.

**Example Usage:**

```c
#include "emitter.h"
#include "ast.h"

// Assuming 'parse_program' is a function that parses tokens into an AST
DeclVec program = parse_program(tokens);
check_program(&program); // Perform checks on the AST

char *c_code = emit_program(program);
// Write c_code to file or compile directly
free(c_code);
```

### `char *emit_hot_split(DeclVec program, const char *hot_lib, char **hot_c_out)`

**Purpose:**
Generates two separate C files for hot-swapping support. One file contains the core logic and hot reload infrastructure, while the other file includes functions marked for hot-loading. This function is useful in development scenarios where you want to update a library without restarting your main program.

**Arguments:**
- `program`: The validated AST representing the main module.
- `hot_lib`: The name or path of the hot-swappable library.
- `hot_c_out`: A pointer that, if set, will be filled with dynamically allocated C code for the hot-swappable library. The caller is responsible for freeing this string.

**Return Value:**
A dynamically allocated string containing the main module's C code. The caller should free this string with `free(main_c)` to avoid memory leaks. The generated library code is returned via the `hot_c_out` pointer and does not need manual allocation or deallocation by the caller, but must be freed separately if required.

**Example Usage:**

```c
#include "emitter.h"
#include "ast.h"

// Assuming 'parse_program' is a function that parses tokens into an AST
DeclVec program = parse_program(tokens);
char *main_c_code;
char *lib_c_code;

emit_hot_split(program, "hotlib.so", &main_c_code);
free(main_c_code);

// Write main_c_code to file (or compile it directly)
// Note: The caller is responsible for freeing lib_c_code

main_c_code = main_c_code; // Placeholder
lib_c_code = emit_hot_split(program, "hotlib.so", NULL); // Generate but do not allocate memory
```

## Notes on Use

- Both `emit_program` and `emit_hot_split` generate C code from ASTs validated by the checker phase. The generated C11 code can then be written to a file or compiled directly using a compiler.
- Ensure that necessary headers are included in the output, such as `<stdlib.h>`, etc., based on the actual requirements of your project.

By following this documentation, users will be able to effectively use these functions within their projects to translate ASTs into working C11 code.