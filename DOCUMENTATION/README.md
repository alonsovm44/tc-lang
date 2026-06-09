# TIG Compiler Documentation Index

> Important note: Check `/.gendox/` for more detailed per file documentation.
Welcome to the TIG compiler documentation! This page helps you navigate all available documentation and find what you need.
## Quick Start

If you're new to the TIG compiler, start with these documents in order:
0. **[Gendox](../.gendox)** - Extended documentation repo.
1. **[Main README.md](../README.md)** - Project overview and quick start
2. **[COMPILER_ARCHITECTURE.md](COMPILER_ARCHITECTURE.md)** - How the compiler works
3. **[AST_GUIDE.md](AST_GUIDE.md)** - Understanding the Abstract Syntax Tree
4. **[DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)** - How to modify and extend the compiler
5. **[compiler/README.md](../compiler/README.md)** - Compiler source directory guide

## Documentation by Topic

### Understanding the Compiler

| Document | Topic | Audience |
|----------|-------|----------|
| [COMPILER_ARCHITECTURE.md](COMPILER_ARCHITECTURE.md) | Complete compilation pipeline | Everyone |
| [AST_GUIDE.md](AST_GUIDE.md) | Abstract Syntax Tree structure | Developers |
| [compiler/README.md](../compiler/README.md) | Compiler source code layout | Developers |
| [TIG_LANGUAGE_SPECIFICATION.md](TIG_LANGUAGE_SPECIFICATION.md) | Language grammar and semantics | Language users & developers |

### Development & Contribution

| Document | Topic | Audience |
|----------|-------|----------|
| [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) | Adding features, code style, debugging | Developers |
| [../CONTRIBUTING.md](../CONTRIBUTING.md) | Contributing guidelines | Contributors |

### Error Handling & Reference

| Document | Topic | Audience |
|----------|-------|----------|
| [Errors.md](Errors.md) | All compiler error codes and examples | Everyone |

### Language Documentation

| Document | Topic | Audience |
|----------|-------|----------|
| [spec1.0.0.md](spec1.0.0.md) through [spec2.0.0.md](spec2.0.0.md) | Historical language specifications | Language historians |
| [TIG_LANGUAGE_SPECIFICATION.md](TIG_LANGUAGE_SPECIFICATION.md) | Current language specification | Language users |

## Find Documentation by Task

### "I want to understand how the compiler works"
→ Read [COMPILER_ARCHITECTURE.md](COMPILER_ARCHITECTURE.md)

### "I want to understand the AST"
→ Read [AST_GUIDE.md](AST_GUIDE.md)

### "I want to add a new feature to TIG"
→ Start with [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md), then read the relevant section

### "I want to understand compiler error codes"
→ Read [Errors.md](Errors.md)

### "I want to contribute to the compiler"
→ Read [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) and [../CONTRIBUTING.md](../CONTRIBUTING.md)

### "I'm getting a compiler error and want to fix it"
→ Look up error code in [Errors.md](Errors.md)

### "I want to understand the source code structure"
→ Read [compiler/README.md](../compiler/README.md)

### "I need to debug the compiler"
→ See "Debugging Tips" in [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)

### "I want to understand the TIG language"
→ Read [TIG_LANGUAGE_SPECIFICATION.md](TIG_LANGUAGE_SPECIFICATION.md)

## File Structure

```
TIG Project
├── README.md                          ← Start here for overview
├── CONTRIBUTING.md                    ← Contribution guidelines
├── compiler/
│   ├── README.md                      ← Compiler source guide
│   ├── include/                       ← Header files (well-documented)
│   │   ├── ast.h                      ← AST data structures
│   │   ├── lexer.h                    ← Tokenization interface
│   │   ├── parser.h                   ← Parsing interface
│   │   ├── checker.h                  ← Type checking interface
│   │   ├── emitter.h                  ← Code generation interface
│   │   └── common.h                   ← Utility functions
│   └── src/                           ← Implementation files
│       ├── main.c                     ← Entry point
│       ├── lexer.c                    ← Tokenization
│       ├── parser.c                   ← Parsing
│       ├── checker.c                  ← Type checking
│       ├── emitter.c                  ← Code generation
│       └── ...
├── DOCUMENTATION/                    ← Documentation directory
│   ├── README.md                      ← This file
│   ├── COMPILER_ARCHITECTURE.md       ← How compiler works
│   ├── DEVELOPER_GUIDE.md             ← How to develop
│   ├── AST_GUIDE.md                   ← AST structure
│   ├── Errors.md                      ← Error codes
│   ├── TIG_LANGUAGE_SPECIFICATION.md  ← Language spec
│   └── spec*.md                       ← Historical specs
└── ...
```

## Key Concepts

### Compilation Pipeline
```
.tc source file
    ↓ [Lexer]
Tokens
    ↓ [Parser]
Abstract Syntax Tree (AST)
    ↓ [Checker]
Validated, annotated AST
    ↓ [Emitter]
C11 code
    ↓ [C Compiler]
Binary executable
```

### Data Structures
- **Type**: Type information (i32, ->i32, etc.)
- **Expr**: Expression nodes (literals, operations, calls)
- **Stmt**: Statement nodes (declarations, control flow)
- **Decl**: Top-level declarations (functions, structs, enums)

### Error Handling
- Errors include source location (file, line, column)
- Error codes like E000, E001, etc.
- Pretty-printed diagnostics with colored output
- Compilation continues with best-effort recovery

## Documentation Tips

### Reading the Header Files
Each header file (`include/*.h`) is now fully documented with:
- **Module-level comments**: Explain the component's purpose
- **Type comments**: Describe what each field means
- **Function comments**: Explain arguments, return values, and usage
- **Example code**: Show how to use the APIs

### Understanding Code Changes
When making changes:
1. Start with the header file to understand the interface
2. Read the corresponding source file for implementation
3. Check existing tests in `test.tc` for examples
4. Add comments explaining your changes
5. Update relevant documentation

### Finding Things in the Code
- Use the **compiler/README.md** table to find line counts and files
- Use **DEVELOPER_GUIDE.md** search for specific concepts
- Use **AST_GUIDE.md** to understand data structure examples
- Grep the source code for function names and types

## Getting Started

### For Users
1. Read [../README.md](../README.md) - Project overview
2. Check out [samples/](../samples/) for example programs
3. Read [TIG_LANGUAGE_SPECIFICATION.md](TIG_LANGUAGE_SPECIFICATION.md) for language details

### For Developers
1. Read [COMPILER_ARCHITECTURE.md](COMPILER_ARCHITECTURE.md) - Overall design
2. Read [AST_GUIDE.md](AST_GUIDE.md) - Data structures
3. Read [compiler/README.md](../compiler/README.md) - Source code organization
4. Read [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) - How to develop
5. Start modifying code!

### For Contributors
1. Read [../CONTRIBUTING.md](../CONTRIBUTING.md)
2. Read [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)
3. Pick an issue or feature to work on
4. Write tests for your changes
5. Submit a pull request

## ❓ FAQs

### Q: Where should I look to understand what happens when code is compiled?
**A**: Read [COMPILER_ARCHITECTURE.md](COMPILER_ARCHITECTURE.md) for the big picture, then look at the relevant source files (lexer.c, parser.c, etc.)

### Q: How do I add support for a new language feature?
**A**: Follow the step-by-step guide in [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) under "Adding a New Feature"

### Q: What does a compiler error mean?
**A**: Look up the error code (E001, E002, etc.) in [Errors.md](Errors.md)

### Q: How do I debug the compiler?
**A**: See "Debugging Tips" in [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)

### Q: What are the coding standards?
**A**: See "Code Style Guide" in [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)

### Q: How do I test my changes?
**A**: See "Testing" section in [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)

### Q: Where is the main entry point?
**A**: [compiler/src/main.c](../compiler/src/main.c)

### Q: How do I understand the AST?
**A**: Read [AST_GUIDE.md](AST_GUIDE.md) with examples and detailed explanations

## Documentation Maintenance

### Adding New Documentation
1. Create file in DOCUMENTATION/ directory
2. Update this index (README.md)
3. Add links to related documents
4. Add to appropriate "Find by Task" section

### Updating Existing Documentation
- Keep examples current
- Update when language features change
- Keep architecture description in sync with code
- Link to related documents

### Keeping Code Comments Current
- Header file comments should match implementation
- Update comments when changing function signatures
- Add detailed comments for complex logic
- Use examples to clarify non-obvious behavior

## Important Links

- **Main README**: [README.md](../README.md)
- **Contributing**: [CONTRIBUTING.md](../CONTRIBUTING.md)
- **Language Spec**: [TIG_LANGUAGE_SPECIFICATION.md](TIG_LANGUAGE_SPECIFICATION.md)
- **Error Codes**: [Errors.md](Errors.md)
- **Compiler Source**: [compiler/README.md](../compiler/README.md)

## Need Help?

1. Check the relevant documentation file
2. Search existing code for examples
3. Look at test cases in `test.tc`
4. Check git history for similar changes
5. Read DEVELOPER_GUIDE.md troubleshooting section
6. OPEN a issue thread on GitHub
7. Message to alvemo44@gmail.com (i won't reply immediately but use github if possible)

---

**Last Updated**: 2026-06-05  
**Compiler Version**: 1.3.0  
**Documentation Status**: Comprehensive with inline comments in headers