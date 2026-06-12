Algorithm for tracking memory leaks:

Since all allocations happen inside functions (for the MVP) we ignore strun methods. 

1. We grep all `fn ` instances and save their positions in a list
2. we go to the first fn decl, and continue consuming chars until we hit `{`
3. we keep a scope stack and push to it everytime we find a `{` and pop when we find a `}`, meanwhile we store each char in a string, when the stack reaches 0 we have fully copied the function body into the string
4. ???