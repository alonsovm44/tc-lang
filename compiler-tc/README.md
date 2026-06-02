# TightC Compiler v1.2.3 - "Pseudo" Self-Host Bridge

This is the intermediate version where the compiler is written in TightC syntax but all logic lives in inline C blocks. This bridges the gap between the C-based compiler (v1.2.2) and the fully self-hosted TightC compiler (v1.3.0+).

## Strategy

1. **v1.2.2** (current): Compiler written in C (.c files)
2. **v1.2.3** (this): Compiler written in TC files with C blocks
3. **v1.3.0+**: Gradually replace C blocks with native TightC

## Structure

```
compiler-tc/
├── src/
│   ├── main.tc      - Entry point (was main.c)
│   ├── lexer.tc     - Lexer (was lexer.c)
│   ├── parser.tc    - Parser (was parser.c)
│   ├── checker.tc   - Type checker (was checker.c)
│   ├── emitter.tc   - C code generator (was emitter.c)
│   ├── ast.tc       - AST definitions (was ast.c)
│   ├── common.tc    - Shared utilities (was common.c)
│   └── io.tc        - File I/O (was io.c)
├── include/
│   └── *.h          - Original C headers (for now)
└── README.md        - This file
```

## Build Process

```bash
# Stage 0: Use C-based compiler to build TC-based compiler
../tigc src/*.tc -o tigc123.c
gcc tigc123.c -o tigc123

# Stage 1: TC-based compiler compiles itself
./tigc123 src/*.tc -o tigc123-v2.c
gcc tigc123-v2.c -o tigc123-v2

# Verify: Both should produce identical output
```

## Migration Rules

1. **File naming**: Change `.c` → `.tc`, keep `.h` for now
2. **Function signatures**: Convert to TightC syntax but keep body in C block
3. **Global variables**: Declare in TightC, initialize in C block if needed
4. **Structs/Enums**: Define in TightC where possible
5. **String literals**: Use TightC strings (no change needed, they're compatible)

## Example Conversion

### Before (C)
```c
// lexer.c
Token next_token(Lexer *l) {
    skip_whitespace(l);
    // ... logic ...
    return tok;
}
```

### After (TC with C blocks)
```tc
// lexer.tc
use "common.tc"

fn Token next_token(Lexer l) {
    "C" {
        skip_whitespace(l);
        // ... all original logic ...
        return tok;
    }
}
```

## Gradual Replacement Roadmap

Phase 1: Port all files to TC-with-C (v1.2.3)
- Week 1: main.tc, io.tc, common.tc
- Week 2: lexer.tc
- Week 3: parser.tc
- Week 4: checker.tc, emitter.tc

Phase 2: Replace C blocks with TightC (v1.3.0+)
- Month 2: Replace string handling
- Month 3: Replace file I/O
- Month 4: Replace lexer logic
- Month 5: Replace parser logic
- Month 6: Replace checker/emitter

Phase 3: True self-host
- Month 7+: Remove all C blocks, compile directly

## Success Criteria

- [ ] `tigc123` can compile any `.tc` file that `tigc122` can
- [ ] `tigc123` can compile itself
- [ ] Output is byte-for-byte identical to C compiler version
- [ ] All tests pass
