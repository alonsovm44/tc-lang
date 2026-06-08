<!-- gendox-provenance
model_id: qwen2.5:3b-instruct
prompt_hash: 11a96d9624d16042
timestamp: 2026-06-07T22:37:42Z
tool_version: gendox v0.2.0
base_commit: c2bfc131acd8472fd41872b4e81819bd80ab7528
-->

# Calculator Program Documentation

## Overview
This program acts as a simple Command Line Interface (CLI) calculator. It accepts user input specifying the type of calculation to perform on two operands, and outputs the result accordingly.

### Usage
The basic syntax for invoking this CLI calculator is:
```
calc <operation> <operand1> <operand2>
```

- `<operation>` can be: `add`, `sub`, `mul`, or `div`.
  
- `<operand1>` represents the first integer number to be used in the calculation.
  
- `<operand2>` represents the second integer number to be used in the calculation.

The program is designed to handle basic arithmetic operations and will provide feedback for invalid inputs and errors such as division by zero.

## Environment Setup
This program utilizes standard libraries:
- `stdlib/io.tc` - For input/output functionality, although it's not explicitly referenced here.
- `stdlib/str.tc` - For string related functionalities including comparison (`strcmp`) and conversion to integer (`atoi`).

### Dependencies
Ensure that the necessary environment for these stdlib modules is properly set up. The program itself doesn't include logic for importing or loading these libraries; they should be part of your overall system setup.

## Function: `main`
- **Function Definition**: The main function serves as the entry point to execute the calculation operations.
  
- **Arguments**:
  - `args`: An array of strings representing the command-line arguments passed to this program. These include the operation type and the operands (`<operand1>` and `<operand2>`).

## Execution Behavior

### Parsing Command-Line Arguments
The function first checks if there are at least four elements in the `args` array, as it expects an operation followed by two operands.

- If less than four elements, it prints usage instructions and exits with status code 1.
  
- Otherwise, it initializes a simple welcome message followed by a prompt asking for input. It then counts how many elements are in `args`.

### Determining Operation Type
The program extracts the first argument (`op`) to determine what operation is required.

#### Addition (`add`):
- If the extracted string matches `"add"`, both operands are converted into integers and added together.
  
- The result of this sum (an integer) is then printed.

#### Subtraction (`sub`):
- For a match with `"sub"`, the first operand (`op1`) is subtracted from the second operand (`op2`), also resulting in an integer.
  
- This difference is then displayed.

#### Multiplication (`mul`):
- In case of a `"mul"` operation, both operands are multiplied to produce another integer result, which gets printed.

#### Division (`div`):
- If the user requests division (`"div"`), the program needs to handle floating point numbers. Both `op1` and `op2` are converted into floats before performing the division.
  
- The resulting float value is then formatted using `%f` for display purposes, ensuring precision.

#### Error Handling:
If any operand cannot be parsed as an integer or if a divisor of zero is attempted (`op2 == 0`), specific error messages are displayed and exit status code 1 is returned to signal failure. 

### Other Operation Types
Any other operation type (not matching `"add"`, `"sub"`, `"mul"`, or `"div"`) results in an invalid input error message, printing "Invalid operation.", followed by an exit status code of 1.

## Return Value
The main function returns the status code `0` to indicate successful execution; a return value of `1` indicates failure due to incorrect operation type, division by zero, or other errors.