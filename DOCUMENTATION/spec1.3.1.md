# v1.3.1

## Try/Catch blocks for error handling
Simple try/catch blocks for error handling, no complex error types or traits.
```tc
try {
    // Code that might throw
    int result = divide(10, 0);
} catch (error) {
    // Handle error
    print("Error: ", error);
}
```