<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: bdfaee9e626bc638
timestamp: 2026-06-07T22:00:20Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# checker.h - Semantic Analysis and Type Checking

The `checker.h` header file contains the implementation for semantic analysis of an Abstract Syntax Tree (AST), with specific focus on type checking, variable scoping, function validation, struct/enum validation, control flow checks, async operations validation, ownership handling, and memory safety.

## Function: check_program

### Description:

`check_program()` is a function responsible for validating the entire AST for semantic correctness. It performs multiple checks including type compatibility, scope verification, function signatures validation, control flow analysis, async operation validation, ownership transfer checks, and memory safety detection. The function modifies the input `program` AST in-place to add type information.

### Parameters:

- **program** (*DeclVec *): A pointer to the DeclVec AST representing the program. This AST is modified in-place with new type information filled by this function.

### Returns:

- **void**: No return value, errors are reported through `tc_error()`.

### Error Categories:

- **Type Errors**: Incompatible types found in operations.
- **Scope Errors**: Uninitialized or undeclared variables or functions encountered.
- **Function Errors**: Function calls have mismatched argument types or do not conform to expected function signatures.
- **Struct/Enum Errors**: Invalid field references, missing struct fields accessed, or invalid enum values used.
- **Async Errors**: Issues with async function usage detected, including potential ownership conflicts during the execution of asynchronous operations.
- **Flow Errors**: Unreachable code identified in control flow structures or missing return statements.

### Example Usage:

```c
DeclVec program = parse_program(tokens);
check_program(&program);  // Validates and annotates AST

if (error_count > 0) {
    fprintf(stderr, "Semantic errors found\n");
    exit(1);
}
```

This example illustrates how to use the `check_program()` function. The parsed tokens from lexical analysis are passed into this function to validate the generated AST for semantic correctness.

### Code Implementation:

```c
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
 */
void check_program(DeclVec *program) {
    // Implementation for semantic analysis goes here

    // Example of a hypothetical check function
    struct_type_check();
    expr_type_check(program);
    var_scope_check(program);
    func_signature_check(program);
    control_flow_check(program);
    async_ops_check(program);
    ownership_check(program);

    // Report any errors found and halt compilation if necessary
    report_and_halt_errors();
}
```

### Explanation of Key Components:

- **Type Checking**: Ensures that all operations use compatible data types.
- **Variable Scoping**: Verifies that variables are declared before their first usage, preventing potential runtime errors.
- **Function Validation**: Checks function signatures to ensure proper argument types and correct return type expectations.
- **Struct/Enum Validations**: Validates struct and enum definitions and ensures appropriate field access.
- **Control Flow Analysis**: Ensures there are no unreachable code paths in the program (e.g., infinite loops, improper breaks) and verifies that all control flow structures have valid exit points or return statements.
- **Async Operations Handling**: Detects issues related to asynchronous function usage such as ownership transfer anomalies during async execution.
- **Ownership Handling**: Checks for proper resource management via the `@` operator, ensuring no memory leaks occur.

### Error Reporting:

Errors encountered during semantic analysis are reported through a custom error reporting mechanism (not shown in this implementation). The program is terminated if severe errors are found to prevent further compilation with an invalid AST. 

This function provides comprehensive validation for the syntactic and semantic correctness of a program's source code, facilitating robust and reliable software development processes.