#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)


typedef struct { char * *ptr; size_t len; } tc_fat_ptr;

typedef enum {
    MOV_RIGHT,
    MOV_LEFT,
    ADD,
    SUB,
    OPEN_LOOP,
    CLOSE_LOOP,
    STDOUT,
    STDIN
} TOKENS;

TOKENS *makeProgramArray(FILE *f);
uint8_t *initTape(uint8_t *tape);
int32_t main(int argc, char **argv);


TOKENS *makeProgramArray(FILE *f) {
    TOKENS *tokens = TC_ALLOC(TOKENS, 2048);
    int32_t c = {0};
    int32_t i = {0};
    while ((!eof(f))) {
        (c = fgetc(f));
        if (ceq(c, 60)) {
            (tokens[i] = 60);
        } else if (ceq(c, 62)) {
            (tokens[i] = 62);
        } else if (ceq(c, 43)) {
            (tokens[i] = 43);
        } else if (ceq(c, 45)) {
            (tokens[i] = 45);
        } else if (ceq(c, 91)) {
            (tokens[i] = 91);
        } else if (ceq(c, 93)) {
            (tokens[i] = 93);
        } else if (ceq(c, 46)) {
            (tokens[i] = 46);
        } else if (ceq(c, 44)) {
            (tokens[i] = 44);
        }
    }
    
    return tokens;
}

uint8_t *initTape(uint8_t *tape) {
    int32_t i = 0;
    while ((i <= 1024)) {
        (tape[i] = 0);
        (i++);
    }
    
    return tape;
}

int32_t main(int argc, char **argv) {
    tc_fat_ptr args = { .ptr = argv, .len = (size_t)argc };
    if ((args.len < 2)) {
        print("usage: tigbf <file.bf>");
    }
    char *source = args.ptr[1];
    uint8_t *tape = TC_ALLOC(uint8_t, 1024);
    initTape(tape);
    FILE *f = openf(source);
    TOKENS *program = makeProgramArray(f);
    int32_t i = 0;
    while ((i <= 1024)) {
        (i++);
        switch (program[i]) {
            case MOV_RIGHT:
                (tape++);
                break;
            case MOV_LEFT:
                (tape--);
                break;
            case ADD:
                (tape[i]++);
                break;
            case SUB:
                (tape[i]--);
                break;
            case OPEN_LOOP:
                break;
            case CLOSE_LOOP:
                break;
            case STDOUT:
                break;
            case STDIN:
                (program.ptr[i] = readc());
                break;
            default:
                break;
        }
    }
    closef(f);
    free(tape);
}
