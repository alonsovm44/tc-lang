# tig

run `vsce package` to build the syntax highlight extension.

# new version 10 june

New Features Added:
Queue/Stack methods - push, pop, peek, enq, deq, size, clear, len

Compound assignments - +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=

Binary/Hex numbers - 0b1010, 0xFF

Character literals - 'a', '\n'

Struct fields - Proper highlighting for struct members

Enum members - Constants in enums

Union fields - The & prefix in structs

Defer statements - Both block defer {} and single-line defer free(ptr)

Pin statements - Highlight pinned variables

Await keyword - For async/await

Use directive - use "stdlib/io.tc"

Array types - i32[8][8]

Inline C block - "C"{ ... }

4. Better Pattern Matching:
Wildcard _ in match statements

Range detection preparation