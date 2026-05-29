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
     "`tcc` requires a source file as its first argument.\n"
     "\n"
     "Usage:\n"
     "    tcc <input.tc> [-o output.c] [-c binary]\n"},
    {"E101", "Missing argument after flag",
     "The `-o` and `-c` flags require a path argument after them.\n"
     "\n"
     "Bad:\n"
     "    tcc main.tc -o\n"
     "\n"
     "Fix:\n"
     "    tcc main.tc -o main.c\n"},
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
     "generated by tcc was rejected. This may indicate a compiler\n"
     "bug. Check the C compiler output above for details.\n"},
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
    fprintf(stderr, "use 'tcc --error list' to see all error codes\n");
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
    static const char *compilers[] = {"gcc", "clang", "cc", "tcc", NULL};
    for (int i = 0; compilers[i]; i++) {
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "%s --version >%s 2>%s", compilers[i], NULL_DEV, NULL_DEV);
        if (system(cmd) == 0) return compilers[i];
    }
    return NULL;
}

int main(int argc, char **argv) {
    const char *input = NULL;
    const char *output = NULL;
    const char *compile_out = NULL;
    const char *hot_lib = NULL;
    bool hot_rebuild = false;
    bool keep_temp = false;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            puts("Tight-C compiler v1.2.0\n"
                 "\n"
                 "Usage: tcc <input.tc> [options]\n"
                 "\n"
                 "Options:\n"
                 "  -o, --output <file>    Write transpiled C to file (.h gets #pragma once)\n"
                 "  -c, --compile <name>   Compile to binary (auto-detects gcc/clang)\n"
                 "  -H, --hot <lib>        Enable hot reload, emit shared library to <lib>\n"
                 "  --hot-rebuild          Rebuild only hot library (for running program)\n"
                 "  -t, --temp             Keep temporary .c files for debugging\n"
                 "  -h, --help             Show this help message\n"
                 "  -v, --version          Show version\n"
                 "  --error <code>         Explain an error code (e.g. --error E000)\n"
                 "  --error list           List all error codes\n"
                 "\n"
                 "Examples:\n"
                 "  tcc main.tc -o main.c           Transpile to C\n"
                 "  tcc main.tc -c app               Transpile + compile to binary\n"
                 "  tcc main.tc -o main.c -c app     Keep .c and compile\n"
                 "  tcc lib.tc -o lib.h               Emit as header\n"
                 "  tcc main.tc -H hot.so -c app     Hot reload: emit lib + host\n");
            return 0;
        } else if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")) {
            puts("tcc 1.2.0");
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
        } else if (!strcmp(argv[i], "-H") || !strcmp(argv[i], "--hot")) {
            if (++i >= argc) die("missing library name after -H");
            hot_lib = argv[i];
        } else if (!strcmp(argv[i], "--hot-rebuild")) {
            hot_rebuild = true;
        } else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--temp")) {
            keep_temp = true;
        } else {
            input = argv[i];
        }
    }
    if (!input) die("usage: tcc <input.tc> [-o output.c] [-c binary]\n       tcc --help for more info");
    char *source = read_file(input);
    tc_set_source(input, source);
    g_current_input = input;
    TokenVec tokens = lex_source(source);
    DeclVec program = parse_program(tokens.items);
    check_program(&program);

    char *c_code = NULL;
    char *hot_c = NULL;
    if (hot_lib) {
        if (hot_rebuild) {
            // Rebuild only hot library
            char *hot_c_only = NULL;
            emit_hot_split(program, hot_lib, &hot_c_only);
            char hot_c_path[256];
            snprintf(hot_c_path, sizeof(hot_c_path), "%s.c", hot_lib);
            write_file(hot_c_path, hot_c_only);

            const char *cc = find_cc();
            if (!cc) die("error: no C compiler found (tried gcc, clang, cc)");

            char hot_cmd[1024];
            char temp_dll[256];
            char target_dll[256];
#ifdef _WIN32
            snprintf(temp_dll, sizeof(temp_dll), "%s_new.dll", hot_lib);
            snprintf(target_dll, sizeof(target_dll), "%s.dll", hot_lib);
            snprintf(hot_cmd, sizeof(hot_cmd), "%s \"%s\" -std=c11 -shared -o \"%s\"", cc, hot_c_path, temp_dll);
#else
            snprintf(temp_dll, sizeof(temp_dll), "%s_new.so", hot_lib);
            snprintf(target_dll, sizeof(target_dll), "%s.so", hot_lib);
            snprintf(hot_cmd, sizeof(hot_cmd), "%s \"%s\" -std=c11 -shared -fPIC -o \"%s\"", cc, hot_c_path, temp_dll);
#endif
            printf("  %s\n", hot_cmd);
            int hot_ret = system(hot_cmd);
            if (hot_ret != 0) die("error: hot library compilation failed (exit %d)", hot_ret);

#ifdef _WIN32
            // On Windows, rename the temp DLL to the actual name (works even if file is loaded)
            if (!MoveFileExA(temp_dll, target_dll, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
                DWORD err = GetLastError();
                fprintf(stderr, "MoveFileExA failed with error %lu\n", err);
                fprintf(stderr, "  temp_dll: %s\n", temp_dll);
                fprintf(stderr, "  target_dll: %s\n", target_dll);
                remove(temp_dll);
                die("error: failed to replace hot library (close the running app)");
            }
            printf("  replaced %s with %s\n", target_dll, temp_dll);
#else
            // On Unix, just rename
            if (rename(temp_dll, target_dll) != 0) {
                remove(temp_dll);
                die("error: failed to replace hot library");
            }
#endif

            if (!keep_temp) remove(hot_c_path);
            printf("  hot library rebuilt: %s\n", hot_lib);
            return 0;
        }
        c_code = emit_hot_split(program, hot_lib, &hot_c);
    } else {
        c_code = emit_program(program);
    }

    if (compile_out) {
        const char *cc = find_cc();
        if (!cc) die("error: no C compiler found (tried gcc, clang, cc)");

        if (hot_lib) {
            // Hot reload mode: write host and hot library
            const char *host_c = output ? output : "__tcc_host.c";
            char hot_c_path[256];
            snprintf(hot_c_path, sizeof(hot_c_path), "%s.c", hot_lib);
            write_file(host_c, c_code);
            write_file(hot_c_path, hot_c);

            // Compile hot library as shared library
            char hot_cmd[1024];
#ifdef _WIN32
            snprintf(hot_cmd, sizeof(hot_cmd), "%s \"%s\" -std=c11 -shared -o \"%s.dll\"", cc, hot_c_path, hot_lib);
#else
            snprintf(hot_cmd, sizeof(hot_cmd), "%s \"%s\" -std=c11 -shared -fPIC -o \"%s\"", cc, hot_c_path, hot_lib);
#endif
            printf("  %s\n", hot_cmd);
            int hot_ret = system(hot_cmd);
            if (hot_ret != 0) die("error: hot library compilation failed (exit %d)", hot_ret);

            // Compile host executable
            char host_cmd[1024];
#ifdef _WIN32
            snprintf(host_cmd, sizeof(host_cmd), "%s \"%s\" -std=c11 -lm -L. -l%s -o \"%s\"", cc, host_c, hot_lib, compile_out);
#else
            snprintf(host_cmd, sizeof(host_cmd), "%s \"%s\" -std=c11 -lm -L. -l%s -o \"%s\"", cc, host_c, hot_lib, compile_out);
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
            const char *tmp_c = output ? output : "__tcc_tmp.c";
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
            snprintf(cmd, sizeof(cmd), "%s \"%s\" -std=c11 -lm -o \"%s\"", cc, tmp_c, compile_out);
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
