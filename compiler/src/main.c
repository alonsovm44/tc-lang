#include "common.h"
#include "checker.h"
#include "emitter.h"
#include "io.h"
#include "lexer.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

const char *g_current_input = NULL;

typedef struct { const char *code; const char *title; const char *explanation; } ErrorInfo;

static const ErrorInfo error_table[] = {
    {"E000", "Assignment to pinned variable",
     "A variable marked with `pin` is immutable in the current scope.\n"
     "You cannot reassign it with `=`, `+=`, `-=`, or any other assignment.\n"
     "\n"
     "Bad:\n"
     "    i32 x = 10\n"
     "    pin x\n"
     "    x = 11       // error: cannot assign to pinned variable\n"
     "\n"
     "Fix: remove the `pin` or avoid reassigning the variable.\n"},
    {"E001", "Expected token not found",
     "The parser expected a specific token (e.g. `{`, `}`, `)`, `:`)\n"
     "but found something else instead.\n"
     "\n"
     "Bad:\n"
     "    i32 fn add i32 a {    // missing ':' after function name\n"
     "\n"
     "Fix:\n"
     "    i32 fn add: i32 a {\n"},
    {"E002", "Expected identifier",
     "The parser expected a variable or function name but found a\n"
     "non-identifier token such as a number or symbol.\n"
     "\n"
     "Bad:\n"
     "    i32 123 = 5\n"
     "\n"
     "Fix: use a valid name starting with a letter or underscore.\n"},
    {"E003", "Keyword used as identifier",
     "A reserved keyword was used where a variable or function name\n"
     "is expected. Keywords like `loop`, `if`, `ret`, `pin`, `defer`\n"
     "cannot be used as identifiers.\n"
     "\n"
     "Bad:\n"
     "    i32 loop = 5\n"
     "\n"
     "Fix: choose a different name that is not a keyword.\n"},
    {"E004", "Expected expression",
     "The parser expected an expression (a value, variable, or operation)\n"
     "but found an unexpected token instead.\n"
     "\n"
     "Bad:\n"
     "    i32 x = }\n"
     "\n"
     "Fix: provide a valid expression on the right side of `=`.\n"},
    {"E005", "Bare identifier statement",
     "A lone identifier on a line is not a valid statement.\n"
     "Identifiers must be part of an assignment, call, or expression.\n"
     "\n"
     "Bad:\n"
     "    x\n"
     "\n"
     "Fix: did you mean `x = ...`, `x()`, or `printi(x)`?\n"},
    {"E006", "Bare literal statement",
     "A lone literal value on a line has no effect and is not allowed.\n"
     "\n"
     "Bad:\n"
     "    42\n"
     "\n"
     "Fix: assign the value to a variable or pass it to a function.\n"},
    {"E007", "Invalid @ directive",
     "The `@` symbol at the top level must be followed by `use`.\n"
     "\n"
     "Bad:\n"
     "    @import \"lib.tc\"\n"
     "\n"
     "Fix:\n"
     "    @use \"lib.tc\"\n"},
    {"E008", "Invalid @use path",
     "`@use` expects a quoted string path to a .tc file.\n"
     "\n"
     "Bad:\n"
     "    @use lib\n"
     "\n"
     "Fix:\n"
     "    @use \"lib.tc\"\n"},
    {"E009", "File not found (@use)",
     "The file specified in `@use` could not be opened.\n"
     "Check that the path is correct and relative to the source file.\n"
     "\n"
     "Bad:\n"
     "    @use \"missing.tc\"\n"
     "\n"
     "Fix: verify the file exists at the given path.\n"},
    {"E010", "Invalid use path",
     "`use` expects a quoted string path to a .tc or .h file.\n"
     "\n"
     "Bad:\n"
     "    use lib\n"
     "\n"
     "Fix:\n"
     "    use \"stdlib/io.tc\"\n"},
    {"E011", "Invalid extern block",
     "`extern` blocks only support the \"C\" ABI.\n"
     "\n"
     "Bad:\n"
     "    extern \"Rust\" { }\n"
     "\n"
     "Fix:\n"
     "    extern \"C\" { }\n"},
    {"E012", "Unterminated literal",
     "A block comment `/* ... */` or string literal `\"...\"` was opened\n"
     "but never closed before end of file or newline.\n"
     "\n"
     "Bad:\n"
     "    /* this comment never ends\n"
     "    i32 x = \"hello\n"
     "\n"
     "Fix: close the comment with `*/` or terminate the string with `\"`.\n"},
    {"E014", "Undefined variable",
     "A variable was used that has not been declared in the current scope\n"
     "or any enclosing scope. Variables declared inside a block `{ }` are\n"
     "not accessible outside of it.\n"
     "\n"
     "Bad:\n"
     "    {\n"
     "        i32 x = 5\n"
     "    }\n"
     "    printi(x)   // error: x is out of scope\n"
     "\n"
     "Fix: declare the variable in the outer scope before the block.\n"},
    {"E013", "Unexpected character",
     "The lexer encountered a character that is not part of the Tight-C\n"
     "grammar. Only ASCII letters, digits, common operators, and\n"
     "punctuation are allowed.\n"
     "\n"
     "Fix: remove the unexpected character from your source file.\n"},
    {"E100", "Missing input file",
     "`tigc` requires a source file as its first argument.\n"
     "\n"
     "Usage:\n"
     "    tigc <input.tc> [-o output.c] [-c binary]\n"},
    {"E101", "Missing argument after flag",
     "The `-o` and `-c` flags require a path argument after them.\n"
     "\n"
     "Bad:\n"
     "    tigc main.tc -o\n"
     "\n"
     "Fix:\n"
     "    tigc main.tc -o main.c\n"},
    {"E102", "Cannot open file",
     "The input source file could not be read. Check that the file\n"
     "exists and you have read permissions.\n"},
    {"E103", "No C compiler found",
     "The `-c` flag requires a C compiler on your PATH.\n"
     "The compiler tries: gcc, clang, cc.\n"
     "\n"
     "Fix: install gcc or clang and make sure it is in your PATH.\n"},
    {"E104", "C compilation failed",
     "The underlying C compiler returned an error. The C output\n"
     "generated by tigc was rejected. This may indicate a compiler\n"
     "bug. Check the C compiler output above for details.\n"},
    {"E015", "Too few arguments to function",
     "A function was called with fewer arguments than it requires.\n"
     "\n"
     "Bad:\n"
     "    i32 fn add: i32 a, i32 b { ret a + b }\n"
     "    add(5)   // error: expected 2 arguments, got 1\n"
     "\n"
     "Fix: provide the correct number of arguments.\n"},
    {"E016", "Too many arguments to function",
     "A function was called with more arguments than it accepts.\n"
     "\n"
     "Bad:\n"
     "    i32 fn add: i32 a, i32 b { ret a + b }\n"
     "    add(1, 2, 3)   // error: expected 2 arguments, got 3\n"
     "\n"
     "Fix: remove the extra arguments or use a variadic function.\n"},
    {"E017", "Owned variable must be passed with @",
     "Owned variables (pointers) must be explicitly marked with the @ operator\n"
     "when passed to functions to indicate ownership transfer.\n"
     "\n"
     "Bad:\n"
     "    fn void consume: i32* p { }\n"
     "    i32* x = alloc(i32)\n"
     "    consume(x)   // error: owned variable must be passed with @\n"
     "\n"
     "Fix:\n"
     "    consume(@x)\n"},
    {NULL, NULL, NULL}
};

static void explain_error(const char *code) {
    const char *CYAN  = "\033[1;36m";
    const char *BOLD  = "\033[1m";
    const char *RESET = "\033[0m";
    for (int i = 0; error_table[i].code; i++) {
        if (!strcmp(error_table[i].code, code)) {
            printf("%s%s%s%s: %s%s\n", CYAN, BOLD, error_table[i].code, RESET, BOLD, error_table[i].title);
            printf("%s\n", RESET);
            printf("%s\n", error_table[i].explanation);
            return;
        }
    }
    fprintf(stderr, "error: unknown error code '%s'\n", code);
    fprintf(stderr, "use 'tigc --error list' to see all error codes\n");
    exit(1);
}

static void list_errors(void) {
    const char *CYAN  = "\033[1;36m";
    const char *RESET = "\033[0m";
    for (int i = 0; error_table[i].code; i++) {
        printf("  %s%s%s  %s\n", CYAN, error_table[i].code, RESET, error_table[i].title);
    }
}

#ifdef _WIN32
#define NULL_DEV "NUL"
#else
#define NULL_DEV "/dev/null"
#endif

static const char *find_cc(void) {
    static const char *compilers[] = {"gcc", "clang", "cc", "tigc", NULL};
    for (int i = 0; compilers[i]; i++) {
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "%s --version >%s 2>%s", compilers[i], NULL_DEV, NULL_DEV);
        if (system(cmd) == 0) return compilers[i];
    }
    return NULL;
}

static bool is_runtime_type(Type *t) {
    if (!t) return false;
    if (t->kind == TY_QUEUE || t->kind == TY_STACK) return true;
    if (t->kind == TY_RAWPTR || t->kind == TY_FATPTR || t->kind == TY_ARRAY || t->kind == TY_FNPTR) {
        if (is_runtime_type(t->inner)) return true;
    }
    if (t->kind == TY_FNPTR) {
        for (int i = 0; i < t->params.count; i++) {
            if (is_runtime_type(t->params.items[i].type)) return true;
        }
    }
    return false;
}

static bool stmt_uses_runtime(Stmt *s) {
    if (!s) return false;
    if (s->type && is_runtime_type(s->type)) return true;
    if (s->expr && s->expr->type && is_runtime_type(s->expr->type)) return true;
    if (s->expr2 && s->expr2->type && is_runtime_type(s->expr2->type)) return true;
    for (int i = 0; i < s->body.count; i++) if (stmt_uses_runtime(s->body.items[i])) return true;
    for (int i = 0; i < s->elseifs.count; i++) {
        if (s->elseifs.items[i].cond && s->elseifs.items[i].cond->type && is_runtime_type(s->elseifs.items[i].cond->type)) return true;
        for (int j = 0; j < s->elseifs.items[i].body.count; j++) if (stmt_uses_runtime(s->elseifs.items[i].body.items[j])) return true;
    }
    for (int i = 0; i < s->else_body.count; i++) if (stmt_uses_runtime(s->else_body.items[i])) return true;
    return false;
}

static bool decl_uses_runtime(Decl *d) {
    if (!d) return false;
    if (d->type && is_runtime_type(d->type)) return true;
    if (d->kind == DC_FN) {
        for (int i = 0; i < d->params.count; i++) if (is_runtime_type(d->params.items[i].type)) return true;
        for (int i = 0; i < d->body.count; i++) if (stmt_uses_runtime(d->body.items[i])) return true;
    }
    return false;
}

static char *get_stdlib_path(void) {
    // 1. Check environment variable
    const char *env_path = getenv("TIG_STDLIB_PATH");
    if (env_path && env_path[0]) return xstrdup(env_path);

    // 2. Check relative to executable
#ifdef _WIN32
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    // Get directory of executable
    char *last_slash = strrchr(exe_path, '\\');
    if (last_slash) {
        *last_slash = '\0';
        char *stdlib_path = malloc(strlen(exe_path) + 16);
        sprintf(stdlib_path, "%s\\stdlib", exe_path);
        return stdlib_path;
    }
#else
    char exe_path[PATH_MAX];
    if (readlink("/proc/self/exe", exe_path, PATH_MAX) != -1) {
        char *last_slash = strrchr(exe_path, '/');
        if (last_slash) {
            *last_slash = '\0';
            char *stdlib_path = malloc(strlen(exe_path) + 16);
            sprintf(stdlib_path, "%s/stdlib", exe_path);
            return stdlib_path;
        }
    }
#endif

    // 3. Default to current directory + stdlib
    return xstrdup("stdlib");
}

typedef struct {
    char **items;
    int count;
    int cap;
} StringVec;

static void string_push(StringVec *v, const char *s) {
    if (v->count == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 8;
        v->items = xrealloc(v->items, sizeof(char*) * (size_t)v->cap);
    }
    v->items[v->count++] = xstrdup(s);
}

int main(int argc, char **argv) {
    StringVec inputs = {0};
    const char *output = NULL;
    const char *compile_out = NULL;
    const char *hot_lib = NULL;
    const char *stdlib_path = NULL;
    bool hot_rebuild = false;
    bool keep_temp = false;
    bool debug_ast = false;
    bool debug_c = false;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            puts("Tight-C compiler v1.3.2\n"
                 "\n"
                 "Usage: tigc <input.tc> [input2.tc ...] [options]\n"
                 "\n"
                 "Options:\n"
                 "  -o, --output <file>    Write transpiled C to file (.h gets #pragma once)\n"
                 "  -c, --compile <name>   Compile to binary (auto-detects gcc/clang)\n"
                 "  -H <lib>               Enable hot reload, emit shared library to <lib>\n"
                 "  --hot                  Rebuild hot library (while app is running)\n"
                 "  -t, --temp             Keep temporary .c files for debugging\n"
                 "  --debug ast            Output AST for debugging\n"
                 "  --debug c              Output generated C code without compiling\n"
                 "  --stdlib-path <path>   Set stdlib path (default: ./stdlib or relative to exe)\n"
                 "  -h, --help             Show this help message\n"
                 "  -v, --version          Show version\n"
                 "  --error <code>         Explain an error code (e.g. --error E000)\n"
                 "  --error list           List all error codes\n"
                 "\n"
                 "Environment Variables:\n"
                 "  TIG_STDLIB_PATH        Path to stdlib directory\n"
                 "\n"
                 "Examples:\n"
                 "  tigc main.tc -o main.c           Transpile to C\n"
                 "  tigc main.tc -c app              Transpile + compile to binary\n"
                 "  tigc main.tc -o main.c -c app    Keep .c and compile\n"
                 "  tigc lib.tc -o lib.h             Emit as header\n"
                 "  tigc main.tc -c app -H hotlib    Hot reload: create app + hotlib_1.dll\n"
                 "  ./app                            Run the application\n"
                 "  tigc main.tc -H hotlib --hot     Rebuild hotlib (app updates automatically)\n"
                 "  tigc main.tc --debug ast          Output AST for debugging\n"
                 "  tigc main.tc --debug c            Output C code without compiling\n"
                 "  tigc a.tc b.tc -c app            Compile multiple files\n");
            return 0;
        } else if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")) {
            puts("tight-c 1.3.2\nHecho en México");
            return 0;
        } else if (!strcmp(argv[i], "--error") || !strcmp(argv[i], "--explain")) {
            if (++i >= argc) die("missing error code after --error");
            if (!strcmp(argv[i], "list")) { list_errors(); return 0; }
            explain_error(argv[i]);
            return 0;
        } else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--compile")) {
            if (++i >= argc) die("missing binary output path after -c");
            compile_out = argv[i];
        } else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) {
            if (++i >= argc) die("missing output path after -o");
            output = argv[i];
        } else if (!strcmp(argv[i], "-H")) {
            if (++i >= argc) die("missing library name after -H");
            hot_lib = argv[i];
        } else if (!strcmp(argv[i], "--hot")) {
            hot_rebuild = true;
        } else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--temp")) {
            keep_temp = true;
        } else if (!strcmp(argv[i], "--debug")) {
            if (++i >= argc) die("missing debug option after --debug (ast or c)");
            if (!strcmp(argv[i], "ast")) {
                debug_ast = true;
            } else if (!strcmp(argv[i], "c")) {
                debug_c = true;
            } else {
                die("invalid debug option: %s (use 'ast' or 'c')", argv[i]);
            }
        } else if (!strcmp(argv[i], "--stdlib-path")) {
            if (++i >= argc) die("missing path after --stdlib-path");
            stdlib_path = argv[i];
        } else {
            string_push(&inputs, argv[i]);
        }
    }
    if (inputs.count == 0) die("usage: tigc <input.tc> [input2.tc ...] [-o output.c] [-c binary]\n       tigc --help for more info");

    // Get stdlib path if not specified
    if (!stdlib_path) {
        stdlib_path = get_stdlib_path();
    }

    // Parse all input files and merge ASTs
    DeclVec program = {0};
    int main_count = 0;

    for (int i = 0; i < inputs.count; i++) {
        const char *input = inputs.items[i];
        char *source = read_file(input);
        tc_set_source(input, source);
        g_current_input = input;
        TokenVec tokens = lex_source(source);
        
        // Phase 1: Collect types from @use directives
        TypeRegistry reg = {0};
        collect_imported_types(tokens.items, input, &reg);
        
        // Phase 2: Parse with pre-registered types
        DeclVec file_program = parse_program_with_types(tokens.items, input, 
                                                        reg.structs, reg.struct_count,
                                                        reg.enums, reg.enum_count);

        // Check for main function in this file
        for (int j = 0; j < file_program.count; j++) {
            Decl *d = file_program.items[j];
            if (d->kind == DC_FN && strcmp(d->name, "main") == 0) {
                main_count++;
            }
            // Set source file for all declarations
            d->source_file = xstrdup(input);
        }

        // Merge declarations into program
        for (int j = 0; j < file_program.count; j++) {
            decl_push(&program, file_program.items[j]);
        }
    }

    // Linker error: only one main function allowed
    if (main_count > 1) {
        die("linker error: multiple main functions found (main defined in %d files)", main_count);
    }

    // Linker error: check for duplicate symbols across files
    for (int i = 0; i < program.count; i++) {
        Decl *d1 = program.items[i];
        if (!d1->name) continue; // Skip anonymous declarations
        
        for (int j = i + 1; j < program.count; j++) {
            Decl *d2 = program.items[j];
            if (!d2->name) continue;
            
            // Check for duplicate symbols (same name and kind)
            if (strcmp(d1->name, d2->name) == 0 && d1->kind == d2->kind) {
                // Allow extern function declarations to match function definitions
                if (d1->kind == DC_FN && d2->kind == DC_EXTERN_FN) continue;
                if (d1->kind == DC_EXTERN_FN && d2->kind == DC_FN) continue;
                
                die("linker error: duplicate symbol '%s' defined in %s and %s",
                    d1->name, d1->source_file, d2->source_file);
            }
        }
    }

    // Linker error: check for circular dependencies via @use
    // Build dependency graph
    typedef struct {
        char *from;
        char *to;
    } DepEdge;
    DepEdge *deps = NULL;
    int dep_count = 0;
    int dep_cap = 0;
    
    for (int i = 0; i < program.count; i++) {
        Decl *d = program.items[i];
        if (d->kind == DC_USE && d->path) {
            // Find which file this use statement is in
            const char *from_file = d->source_file;
            const char *to_file = d->path;
            
            // Add edge to dependency graph
            if (dep_count == dep_cap) {
                dep_cap = dep_cap ? dep_cap * 2 : 16;
                deps = xrealloc(deps, sizeof(DepEdge) * (size_t)dep_cap);
            }
            deps[dep_count].from = xstrdup(from_file);
            deps[dep_count].to = xstrdup(to_file);
            dep_count++;
        }
    }
    
    // Detect cycles using DFS
    if (dep_count > 0) {
        for (int i = 0; i < inputs.count; i++) {
            const char *start = inputs.items[i];
            
            // Build cycle path for error reporting
            char *path[256];
            int path_len = 0;
            char *visited = calloc(inputs.count + dep_count, sizeof(char));
            
            // Simple cycle detection - follow chains
            const char *current = start;
            path[path_len++] = xstrdup(current);
            
            for (int step = 0; step < dep_count + 10; step++) {
                bool found_edge = false;
                for (int j = 0; j < dep_count; j++) {
                    if (strcmp(deps[j].from, current) == 0) {
                        current = deps[j].to;
                        path[path_len++] = xstrdup(current);
                        found_edge = true;
                        
                        // Check if we're back to start
                        if (strcmp(current, start) == 0) {
                            // Build error message
                            Str cycle_str = {0};
                            for (int k = 0; k < path_len; k++) {
                                str_add(&cycle_str, path[k]);
                                if (k < path_len - 1) str_add(&cycle_str, " -> ");
                            }
                            die("linker error: circular dependency detected: %s", cycle_str.data);
                        }
                        break;
                    }
                }
                if (!found_edge) break;
            }
            
            // Cleanup path
            for (int k = 0; k < path_len; k++) {
                free(path[k]);
            }
            free(visited);
        }
    }
    
    // Clean up dependency graph
    for (int i = 0; i < dep_count; i++) {
        free(deps[i].from);
        free(deps[i].to);
    }
    free(deps);

    check_program(&program);
    
    // Debug: Output AST
    if (debug_ast) {
        printf("// AST Dump\n");
        for (int i = 0; i < program.count; i++) {
            Decl *d = program.items[i];
            printf("// Decl: %s\n", d->name ? d->name : "(anonymous)");
        }
        printf("// End AST Dump\n\n");
    }
    
    // Check if program contains async functions or try/catch
    bool needs_runtime = false;
    for (int i = 0; i < program.count; i++) {
        Decl *d = program.items[i];
        if ((d->kind == DC_FN && d->is_async) || decl_uses_runtime(d)) {
            needs_runtime = true;
            break;
        }
        // Check if function uses try/catch
        if (d->kind == DC_FN && d->body.count > 0) {
            for (int j = 0; j < d->body.count; j++) {
                if (d->body.items[j]->kind == ST_TRY || d->body.items[j]->kind == ST_THROW) {
                    needs_runtime = true;
                    break;
                }
            }
            if (needs_runtime) break;
        }
    }

    char *c_code = NULL;
    char *hot_c = NULL;
    if (hot_lib) {
        if (hot_rebuild) {
            // Rebuild only hot library
            char *hot_c_only = NULL;
            emit_hot_split(program, hot_lib, &hot_c_only, stdlib_path);
            char hot_c_path[256];
            snprintf(hot_c_path, sizeof(hot_c_path), "%s.c", hot_lib);
            write_file(hot_c_path, hot_c_only);

            const char *cc = find_cc();
            if (!cc) die("error: no C compiler found (tried gcc, clang, cc)");

            // Read current version from file, or start at 1
            int version = 1;
            char version_file[256];
            snprintf(version_file, sizeof(version_file), "%s.version", hot_lib);
            FILE *vf = fopen(version_file, "r");
            if (vf) {
                fscanf(vf, "%d", &version);
                fclose(vf);
                version++;
            }
            
            // Write new version
            vf = fopen(version_file, "w");
            if (vf) {
                fprintf(vf, "%d", version);
                fclose(vf);
            }
            printf("  hot library version: %d\n", version);

            char hot_cmd[1024];
            char versioned_dll[256];
#ifdef _WIN32
            snprintf(versioned_dll, sizeof(versioned_dll), "%s_%d.dll", hot_lib, version);
            snprintf(hot_cmd, sizeof(hot_cmd), "%s \"%s\" -std=c11 -shared -o \"%s\"", cc, hot_c_path, versioned_dll);
#else
            snprintf(versioned_dll, sizeof(versioned_dll), "%s_%d.so", hot_lib, version);
            snprintf(hot_cmd, sizeof(hot_cmd), "%s \"%s\" -std=c11 -shared -fPIC -o \"%s\"", cc, hot_c_path, versioned_dll);
#endif
            printf("  %s\n", hot_cmd);
            int hot_ret = system(hot_cmd);
            if (hot_ret != 0) die("error: hot library compilation failed (exit %d)", hot_ret);

            if (!keep_temp) remove(hot_c_path);
            printf("  hot library rebuilt: %s\n", hot_lib);
            return 0;
        }
        c_code = emit_hot_split(program, hot_lib, &hot_c, stdlib_path);
    } else {
        c_code = emit_program(program, stdlib_path);
    }
    
    // Debug: Output C code
    if (debug_c) {
        fputs(c_code, stdout);
        return 0;
    }

    if (compile_out) {
        const char *cc = find_cc();
        if (!cc) die("error: no C compiler found (tried gcc, clang, cc)");

        if (hot_lib) {
            // Hot reload mode: write host and hot library
            const char *host_c = output ? output : "__tigc_host.c";
            char hot_c_path[256];
            snprintf(hot_c_path, sizeof(hot_c_path), "%s.c", hot_lib);
            write_file(host_c, c_code);
            write_file(hot_c_path, hot_c);

            // Initialize version file
            char version_file[256];
            snprintf(version_file, sizeof(version_file), "%s.version", hot_lib);
            FILE *vf = fopen(version_file, "w");
            if (vf) {
                fprintf(vf, "1");
                fclose(vf);
            }

            // Compile hot library as shared library (version 1)
            char hot_cmd[1024];
#ifdef _WIN32
            snprintf(hot_cmd, sizeof(hot_cmd), "%s \"%s\" -std=c11 -shared -o \"%s_1.dll\"", cc, hot_c_path, hot_lib);
#else
            snprintf(hot_cmd, sizeof(hot_cmd), "%s \"%s\" -std=c11 -shared -fPIC -o \"%s_1.so\"", cc, hot_c_path, hot_lib);
#endif
            printf("  %s\n", hot_cmd);
            int hot_ret = system(hot_cmd);
            if (hot_ret != 0) die("error: hot library compilation failed (exit %d)", hot_ret);

            // Compile host executable
            char host_cmd[1024];
#ifdef _WIN32
            snprintf(host_cmd, sizeof(host_cmd), "%s \"%s\" -std=c11 -lm -L. -l%s_1 -o \"%s\"", cc, host_c, hot_lib, compile_out);
#else
            snprintf(host_cmd, sizeof(host_cmd), "%s \"%s\" -std=c11 -lm -lpthread -L. -l%s_1 -o \"%s\"", cc, host_c, hot_lib, compile_out);
#endif
            printf("  %s\n", host_cmd);
            int host_ret = system(host_cmd);
            if (host_ret != 0) die("error: host compilation failed (exit %d)", host_ret);

            if (!output && !keep_temp) {
                remove(host_c);
                remove(hot_c_path);
            }
            printf("  compiled: %s (with hot reload)\n", compile_out);
        } else {
            // Normal mode
            const char *tmp_c = output ? output : "__tigc_tmp.c";
            size_t tlen = strlen(tmp_c);
            bool tmp_is_header = (tlen > 2 && !strcmp(tmp_c + tlen - 2, ".h"));
            if (tmp_is_header) {
                Str wrapped = {0};
                str_add(&wrapped, "#pragma once\n");
                str_add(&wrapped, c_code);
                write_file(tmp_c, wrapped.data);
            } else {
                write_file(tmp_c, c_code);
            }

            char cmd[1024];
            if (needs_runtime) {
                snprintf(cmd, sizeof(cmd), "%s \"%s\" -std=c11 -lm -o \"%s\" \"compiler/src/runtime.o\"", cc, tmp_c, compile_out);
            } else {
                snprintf(cmd, sizeof(cmd), "%s \"%s\" -std=c11 -lm -o \"%s\"", cc, tmp_c, compile_out);
            }
            printf("  %s\n", cmd);
            int ret = system(cmd);

            if (!output && !keep_temp) remove(tmp_c);
            if (ret != 0) die("error: C compilation failed (exit %d)", ret);
            printf("  compiled: %s\n", compile_out);
        }
    } else if (output) {
        size_t olen = strlen(output);
        bool is_header = (olen > 2 && !strcmp(output + olen - 2, ".h"));
        if (is_header) {
            Str wrapped = {0};
            str_add(&wrapped, "#pragma once\n");
            str_add(&wrapped, c_code);
            write_file(output, wrapped.data);
        } else {
            write_file(output, c_code);
        }
    } else {
        fputs(c_code, stdout);
    }
    return 0;
}
