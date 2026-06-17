# Tiger
Tig's build system. The best of Make plus the best of Cargo.

Make: simplicity and battle tested
Cargo: convenience without complexity

Concerns:
    1. Managing tigc invocations (flags, order, parallelism)

    2. Interpreting the tigerfile (dependencies, targets, sources)

    3. Incremental builds (only recompile what changed)

    4. Output directory management (build/ and dist/)

    5. Begin Tig projects

    6. Handle llvm flags

Usage:
    tiger init
    tiger build
    tiger run
