# TightC Self-Hosting Roadmap

## Current State: v1.2.2 (C Compiler)
The compiler is written in C11. It works. It's done.

## v1.2.3: The Bridge (This Directory)
**Goal**: Compiler written in `.tc` files with inline C blocks
**Status**: Skeleton structure created

### Phase 1: Port to TC (Week 1-2)
- [ ] `common.tc` - Shared types and utilities
- [ ] `io.tc` - File I/O operations
- [ ] `main.tc` - Entry point and CLI
- [ ] `lexer.tc` - Tokenizer
- [ ] `parser.tc` - Parser
- [ ] `checker.tc` - Type checker
- [ ] `emitter.tc` - C code generator

### Phase 2: Verify Equivalence (Week 3)
- [ ] TC compiler produces identical output to C compiler
- [ ] All existing tests pass
- [ ] Can compile itself (bootstrap)

### Phase 3: Freeze C Core (Week 4)
- [ ] No more features added to C-based compiler
- [ ] Bug fixes only
- [ ] All development moves to TC-based compiler

## v1.3.0: Gradual Replacement
**Goal**: Replace C blocks with native TightC

### Strategy: Inside-Out Replacement
Start with the "leaf" modules that have no dependencies, work toward the "root" (main).

#### Month 1: String Module
- [ ] Create `std/string.tc` with TightC string operations
- [ ] Replace C string functions in compiler
- [ ] `strlen`, `strcmp`, `strcpy`, `strcat` in TightC

#### Month 2: Memory Module
- [ ] Create `std/alloc.tc` with TightC allocator
- [ ] Replace C `malloc`/`free` in compiler
- [ ] Add arena allocator for compiler use

#### Month 3: I/O Module
- [ ] Create `std/io.tc` with TightC file I/O
- [ ] Replace C `fopen`/`fread`/`fwrite` in compiler
- [ ] Buffered I/O in TightC

#### Month 4: Lexer in TightC
- [ ] Character-by-character tokenization
- [ ] Keyword recognition table
- [ ] Number and string literal parsing
- [ ] Macro preprocessing

#### Month 5: Parser in TightC
- [ ] Recursive descent parsing
- [ ] AST construction
- [ ] Error recovery

#### Month 6: Checker in TightC
- [ ] Type inference (limited)
- [ ] Type checking
- [ ] Scope analysis

#### Month 7: Emitter in TightC
- [ ] C code generation
- [ ] String templating
- [ ] Formatting

#### Month 8: Main in TightC
- [ ] CLI argument parsing
- [ ] Module loading
- [ ] Error reporting

## v1.4.0: True Self-Host
**Goal**: Compiler written in 100% TightC

### Bootstrap Verification
```
tc-v1.2.3 (C) → compiles → tc-v1.4.0 (TC)
tc-v1.4.0 (TC) → compiles → tc-v1.4.0-v2 (TC)
tc-v1.4.0-v2 (TC) → compiles → tc-v1.4.0-v3 (TC)
...all outputs identical...
```

### Cleanup
- [ ] Remove all inline C blocks
- [ ] Remove C standard library dependencies
- [ ] Pure TightC standard library

## v2.0.0: Native Backend (Future)
**Goal**: Compile directly to machine code (no C intermediate)

### Possible Backends
- [ ] LLVM IR
- [ ] Cranelift (Rust's codegen)
- [ ] Custom x86_64 backend
- [ ] WASM

## Key Design Decisions

### Why Inline C First?
1. **Proven strategy**: Used by Go, Rust, Zig, etc.
2. **Incremental**: Replace one function at a time
3. **Testable**: Can verify equivalence at every step
4. **Safe**: Can always fall back to C if needed

### Why Not Direct Port?
1. Too much work at once
2. Hard to verify correctness
3. Risk of introducing bugs during port
4. Demotivating - years of work before it runs

### Order of Replacement
1. **Utilities** (strings, memory) - no dependencies, easy to test
2. **Lexer** - well-defined interface, table-driven
3. **Parser** - recursive descent, natural in TightC
4. **Checker** - complex logic, but manageable
5. **Emitter** - generates C, so we need C working first
6. **I/O** - platform-specific, save for last

## Success Metrics

### v1.2.3
- [ ] TC compiler runs
- [ ] Produces correct output
- [ ] Compiles itself

### v1.3.0
- [ ] 50% of code in native TightC
- [ ] No regressions in test suite
- [ ] Faster compilation (measured)

### v1.4.0
- [ ] 100% TightC
- [ ] Bootstrap verified
- [ ] Performance parity with C version

### v2.0.0
- [ ] Native backend
- [ ] No C toolchain required
- [ ] Competitive performance

## Risks and Mitigations

### Risk: TightC not expressive enough
**Mitigation**: Keep C blocks as escape hatch. Add features to TightC as needed.

### Risk: Performance regression
**Mitigation**: Benchmark at every step. Profile before optimizing.

### Risk: Bootstrap breaks
**Mitigation**: Keep last known good compiler. Can always rebuild from C.

### Risk: Loss of interest
**Mitigation**: Small milestones. Celebrate each module replacement.

## Timeline

- **v1.2.3**: June 2026 (2 weeks)
- **v1.3.0**: December 2026 (6 months)
- **v1.4.0**: June 2027 (6 months)
- **v2.0.0**: 2028+ (research phase)

## Current Status: v1.2.3 Skeleton Complete

Created:
- `README.md` - Strategy document
- `ROADMAP.md` - This file
- `Makefile` - Build system
- `src/common.tc` - Shared types
- `src/lexer.tc` - Tokenizer skeleton
- `src/main.tc` - Entry point skeleton

Next: Fill in the actual C code from existing compiler.
