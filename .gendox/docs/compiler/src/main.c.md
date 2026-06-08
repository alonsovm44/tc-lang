<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: cb2cd676968e96ca
timestamp: 2026-06-07T22:24:39Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# Markdown Documentation for Tight-C Compiler

## Introduction
This document provides a comprehensive guide to the usage and behavior of the Tight-C compiler (`tcc`). The compiler is designed to translate Tight-C source files into C code or binary executables. This documentation covers key concepts, functions, error codes, and options.

## Usage
### Command-Line Options
The `tcc` compiler supports various command-line arguments for controlling its behavior:

- **--help**: Displays help information.
- **-h**: Alias for `--help`.
- **--version**: Shows the version of the compiler.
- **--error <code>**: Explains an error code (e.g., `--error E000`).
- **--error list**: Lists all error codes.
- **-o, --output <file>**: Writes transpiled C code to a file. The output type is inferred from the file extension; `.h` for headers and no extension for regular files.
- **-c, --compile <name>**: Compiles generated C code into an executable named `<name>.<extension>` (where the extension defaults to `c`). If used with `-o`, both outputs are produced.  
  - The compiler checks for a standard C compiler (`gcc`, `clang`, or `cc`) on the system's PATH.
- **--hot**: Enables hot reload mode, which emits code and then compiles it as needed while an application is running. This requires specifying an additional library name with `-H <lib_name>`.
  - **-H <lib_name>**: Specifies a custom shared library name for the hot reload mode to use.
  - **--hot** must be used in conjunction with `-H` and only supports `--hot` as its argument.
- **-t, --temp**: Keeps temporary files during debugging or development process. 
- **--debug ast**: Outputs an Abstract Syntax Tree (AST) for further analysis during development.
- **--debug c**: Outputs the generated C code without compiling it into a binary executable.

### Example Usage
```bash
# Transpile source file to C and output as .c file
tcc main.tc -o

# Compile transpiled C code into an executable (with standard gcc/clang detection)
tcc main.tc -c app

# Hot reload mode, emitting shared library and host executable. 
# Compiles generated C code only once during initial application start.
tcc main.tc --hot -H hotlib
```

## Error Handling
The compiler returns a series of error codes that can be used to understand the failure modes during compilation or transpilation. Here is a list of all defined error codes:

### Error Codes List
```markdown
  # E000: Assignment to pinned variable
  E001: Expected token not found
  E002: Expected identifier
  E003: Keyword used as identifier
  E004: Expected expression
  E005: Bare identifier statement
  E006: Bare literal statement
  E007: Invalid @ directive
  E008: Invalid @use path
  E009: File not found (@use)
  E010: Invalid use path
  E011: Invalid extern block
  E012: Unterminated literal
  E013: Unexpected character
  E014: Undefined variable
  E100: Missing input file
  E101: Missing argument after flag
  E102: Cannot open file
  E103: No C compiler found
  E104: C compilation failed
```

### Explanation of Error Codes

#### Example Error Code: E000 - Assignment to pinned variable
**Description**: A variable marked with `pin` is immutable in the current scope. You cannot reassign it with any form of assignment.
**Example**:
```tightc
i32 x = 10
pin x
x = 11       // error: cannot assign to a pinned variable
```
**Fix**: Remove the `pin` keyword or avoid reassigning the variable.

#### Example Error Code: E009 - File not found (@use)
**Description**: The file specified in `@use` could not be opened. Check that the path is correct and relative to the source file.
**Example**:
```tightc
@use missing.tc   // error: file 'missing.tc' could not be read
```
**Fix**: Verify the file exists at the given path.

### Explanation of Functions

#### Function `explain_error(const char *code)`
This function takes an error code and prints detailed information about it. If no match is found, it provides a helpful message to guide the user towards possible errors or issues with their script.
```c
void explain_error(const char *code) {
    // Implementation details
}
```

#### Function `list_errors()`
Lists all defined error codes in an easily readable format for reference.
```c
void list_errors(void) {
    // Implementation details
}
```

### Environment Configuration

- **`g_current_input`**: A pointer to the current input file being processed, used during tokenization and parsing.
- **Compiler Detection**: The compiler checks for standard C compilers (`gcc`, `clang`, or `cc`) on the system's PATH. If none is found, it returns an error code `E103: No C compiler found`.

## Debugging
The `tcc` compiler supports debugging options such as outputting the Abstract Syntax Tree (AST) for analysis during development (`--debug ast`), or emitting raw generated C code instead of compiling it into a binary executable (`--debug c`).

### Debugging with AST and C Code

- **Outputting AST**: Use `--debug ast` to output an ASCII representation of the abstract syntax tree.
  ```bash
  tcc main.tc --debug ast
  ```
  
- **Outputting Raw C Code**: Use `--debug c` to directly output generated C code without compiling it.
  ```bash
  tcc main.tc --debug c
  ```

### Additional Compiler Configuration Options

The compiler also allows for additional configuration options such as:
- Keeping temporary files (`-t, --temp`)
- Compiling with external libraries (`-H <lib_name>`) and emitting shared library code (hot reload mode).

## Conclusion
This comprehensive guide covers the usage, error handling, and debugging capabilities of the Tight-C compiler. Utilizing these features effectively can help streamline development processes and resolve common issues encountered during the translation process.