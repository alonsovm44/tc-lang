#include "common.h"
#include "emitter.h"
#include "io.h"
#include "lexer.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *input = NULL;
    const char *output = NULL;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) {
            if (++i >= argc) die("missing output path");
            output = argv[i];
        } else {
            input = argv[i];
        }
    }
    if (!input) die("usage: tightc <input.tc> [-o output.c]");
    char *source = read_file(input);
    TokenVec tokens = lex_source(source);
    DeclVec program = parse_program(tokens.items);
    char *c = emit_program(program);
    if (output) write_file(output, c);
    else fputs(c, stdout);
    return 0;
}
