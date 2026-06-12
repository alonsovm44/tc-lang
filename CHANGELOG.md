## v1.3.0/1.3.1 2026-06-04

Added:

- added explicit casting x = (T)y
- added braceless defer for ergonomics
- added argument count validation in tig level, not falling back into C
- added snq.c compilation
- added queue stack much needed built in methods size() clear() isEmpty()
- added putchr() to the IO stdlib
- added runtime async.tc to stdlib
- added async functions with automatic runtime initialization
- added async module with thread pool management
- added wrapper function generation for async calls with arguments
- added automatic runtime linking for async programs
- added proper stdlib organization (async.h in stdlib/)
- added automatic cleanup using atexit() for async resources

Removed:

- removed need for manual async_init() and async_shutdown() calls
- removed debug print statements from runtime stub functions
- removed async.h from current working directory

Improved/Fixed:

- fixed inlining including libs not working, now it looks for the file relative to the file not the project root
- Queue and stacks vars are now automatically destroyed when they go out of scope
- Enforced move semantics, now they apply
- fixed queue stack param passing
- fixed catch arm parsing error
- separated the queue and stack runtime from the async runtime
- fixed fputc() not working
- fixed queues and async not working together
- fixed async function call code generation (removed typeof, added semicolons)
- fixed compiler -c flag to auto-link runtime when async functions detected
- fixed async module integration with proper header includes
- fixed runtime to use lazy initialization instead of manual setup
- improved async programming experience , zero boilerplate needed
- improved code generation for async calls with primitive arguments
- improved stdlib organization and module structure
## v1.2.2 2026-06-02

Added:

Removed:

- removed hot keyword and simplified hot reloading, now works good
Improved/Fixed:

- improved hot reloading and made it easier
## v1.2.1 2026-06-02

Added:
- added methods to struns
- added varargs ... 
- expanded io stdlib to include more file io functions
- added ptr access to struct fields with .> and passing by reference with & (borrowing)

Removed:

Improved/Fixed:

## v1.2.0 2026-05-30

Added:
- added basic macros and parametric macros
- added basic macro support for basic text replacement
- added inline C working in the global scope
- added sizeof()
- added inline C code
- added enums

Removed:

- removed conditional defer
Improved/Fixed:

- changed binary name from tightc to tigc
- changed binary name from tcc to tightc so we dont collide with tiny c compiler (tcc)
- fixed varargs not working>
## v1.1.0 2026-05-22

Added:
- added new modules to stdlib
- added else and if else stmts
- added function pointers for callbacks
- added pattern matching 'match' keyword for modern ergonomics' 
- added anonymous padding to struns
- added struns and strun examples to samples
- added new function syntax 'fn <type> <name>: type arg, ... {}' and it now coexists with old syntax for retro compatibility
- added rust like error help in CLI
- enforced pin comptime error display
- added version and help arguments to compiler cli
    - Documentation files
    - Changelog file


Removed:

Improved/Fixed:

- fixed scoping errors not showing in compile time by tcc
- fixed struns being packed by default, now they can be packed with @
## v1.0.0 2026-05-22
<initial release>
Added:

Removed:

Improved/Fixed:

# Changelog
All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Changelog kept with Chang CLI tool