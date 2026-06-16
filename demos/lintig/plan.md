Algorithm for tracking memory leaks:

Since all allocations happen inside functions (for the MVP) we ignore strun methods. 

We are basically looking for allocations without their corresponding free() stmts, we have to look for them
in defer blocks or manually before returns.

1. 