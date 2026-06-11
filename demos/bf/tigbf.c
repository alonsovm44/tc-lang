#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)
#include "stdlib/async.h"


typedef struct { char * *ptr; size_t len; } tc_fat_ptr;

#include "stdlib/io.h"
#include "stdlib/str.h"
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
void initTape(uint8_t *tape);
void precomputeLoops(TOKENS *program, int32_t len, int32_t *matches);
int32_t main(int argc, char **argv);


TOKENS *makeProgramArray(FILE *f) {
    TOKENS *base = TC_ALLOC(TOKENS, 30*1024);
    int32_t c = {0};
    int32_t i = {0};
    while ((!eof(f))) {
        (c = filegetc(f));
        if (ceq(c, 60)) {
            (base[i] = 1);
        } else if (ceq(c, 62)) {
            (base[i] = 0);
        } else if (ceq(c, 43)) {
            (base[i] = 2);
        } else if (ceq(c, 45)) {
            (base[i] = 3);
        } else if (ceq(c, 91)) {
            (base[i] = 4);
        } else if (ceq(c, 93)) {
            (base[i] = 5);
        } else if (ceq(c, 46)) {
            (base[i] = 6);
        } else if (ceq(c, 44)) {
            (base[i] = 7);
        }
        (i++);
    }
    
    return base;
}

void initTape(uint8_t *tape) {
    int32_t i = 0;
    while ((i < (30 * 1024))) {
        (tape[i] = 0);
        (i++);
    }
}

void precomputeLoops(TOKENS *program, int32_t len, int32_t *matches) {
    Stack loopStack = stack_create(0);
    int32_t stack_size = 0;
    int32_t pc = 0;
    while ((pc < len)) {
        if ((program[pc] == 4)) {
            stack_push(&loopStack, &(int32_t){ pc }, sizeof(int32_t));
            (stack_size++);
        } else if ((program[pc] == 5)) {
            if ((stack_size == 0)) {
                print("Error, unexpected closing bracket\n");
            }
            int32_t match_pos = *(int32_t*)stack_pop(&loopStack);
            (matches[match_pos] = pc);
            (matches[pc] = match_pos);
            (stack_size--);
        }
        (pc++);
    }
    if ((stack_size != 0)) {
        print("Error, unclosed loop\n");
        int32_t i = 0;
        while ((i < stack_size)) {
            print("Error, unclosed loop at position: ");
            print(i);
            (i++);
        }
    }
}

int32_t main(int argc, char **argv) {
    tc_fat_ptr args = { .ptr = argv, .len = (size_t)argc };
    if ((args.len < 2)) {
        print("usage: tigbf <file.bf>");
        
        return 1;
    }
    char *source = args.ptr[1];
    uint8_t *tape_base = TC_ALLOC(uint8_t, (30 * 1024));
    uint8_t *ptr = tape_base;
    initTape(tape_base);
    FILE *f = openf(source, "r");
    TOKENS *program = makeProgramArray(f);
    int32_t *matches = TC_ALLOC(int32_t, 30*1024);
    precomputeLoops(program, 30*1024, matches);
    int32_t i = 0;
    while ((i < 30*1024)) {
        switch (program[i]) {
            case MOV_RIGHT:
                (ptr++);
                break;
            case MOV_LEFT:
                (ptr--);
                break;
            case ADD:
                (*(ptr++));
                break;
            case SUB:
                (*(ptr--));
                break;
            case OPEN_LOOP:
                if (((*ptr) == 0)) {
                    (i = matches[i]);
                }
                break;
            case CLOSE_LOOP:
                if (((*ptr) != 0)) {
                    (i = matches[i]);
                }
                break;
            case STDOUT:
                putchar((*ptr));
                break;
            case STDIN:
                ((*ptr) = readc());
                break;
            default:
                break;
        }
        (i++);
    }
    free(matches);
    free(program);
    closef(f);
    free(tape_base);
}
