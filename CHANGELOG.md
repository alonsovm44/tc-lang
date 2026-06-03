## v1.2.2 2026-06-02

Added:

Removed:

- removed hot keyword and simplified hot reloading, now works good
Improved/Fixed:

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

Added:

Removed:

Improved/Fixed:

# Changelog
All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Changelog kept with Chang CLI tool