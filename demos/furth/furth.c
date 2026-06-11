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

#include "stdlib/io.h"
#include "stdlib/str.h"
#include "stdlib/snq.h"
typedef enum {
    ADD,
    SUB,
    MUL,
    DIV
} OPERAND;

Queue makeProgramQ(FILE *f, int32_t *lengthCounter);
char isSourceFu(char *path);
char isANumber(uint8_t chr);
char isOperand(uint8_t chr);
char isPrintStmt(uint8_t chr);
int32_t main(int argc, char **argv);


Queue makeProgramQ(FILE *f, int32_t *lengthCounter) {
    Queue q = queue_create(0);
    uint8_t c = 0;
    while (1) {
        (c = filegetc(f));
        queue_push(&q, &(uint8_t){ c }, sizeof(uint8_t));
        ((*lengthCounter) = ((*lengthCounter) + 1));
        if (eof(f)) {
            break;
        }
    }
    
    return q;
}

char isSourceFu(char *path) {
    if (strhas(path, ".fu")) {
        
        return 1;
    } else {
        
        return 0;
    }
}

char isANumber(uint8_t chr) {
    if (((chr >= 48) && (chr <= 57))) {
        
        return 1;
    } else {
        
        return 0;
    }
}

char isOperand(uint8_t chr) {
    if ((((ceq(chr, 43) || ceq(chr, 45)) || ceq(chr, 42)) || ceq(chr, 47))) {
        
        return 1;
    } else {
        
        return 0;
    }
}

char isPrintStmt(uint8_t chr) {
    if (ceq(chr, 33)) {
        
        return 1;
    } else {
        
        return 0;
    }
}

int32_t main(int argc, char **argv) {
    tc_fat_ptr args = { .ptr = argv, .len = (size_t)argc };
    Stack globalStack = stack_create(0);
    if ((args.len < 2)) {
        print("Usage:\n furth <source.fu>");
        
        return 1;
    }
    char *src = args.ptr[1];
    if ((!isSourceFu(src))) {
        print("Error, make sure source code is .fu file.");
        
        return 1;
    }
    FILE *f = openf(src, "r");
    int32_t leng = 0;
    Queue program = makeProgramQ(f, &leng);
    int32_t i = 0;
    while ((i < leng)) {
        if ((leng == 0)) {
            break;
        }
        uint8_t chr = *(uint8_t*)queue_pop(&program);
        if ((chr == 59)) {
            while ((i < leng)) {
                (chr = *(uint8_t*)queue_pop(&program));
                (i++);
                if ((chr == 10)) {
                    break;
                }
            }
        } else if (isANumber(chr)) {
            int32_t val = (chr - 48);
            stack_push(&globalStack, &(double){ val }, sizeof(double));
        } else if (isOperand(chr)) {
            double x = *(double*)stack_pop(&globalStack);
            double y = *(double*)stack_pop(&globalStack);
            switch (chr) {
                case 43:
                    stack_push(&globalStack, &(double){ (x + y) }, sizeof(double));
                    break;
                case 45:
                    stack_push(&globalStack, &(double){ (x - y) }, sizeof(double));
                    break;
                case 42:
                    stack_push(&globalStack, &(double){ (x * y) }, sizeof(double));
                    break;
                case 47:
                    if ((y == 0.0)) {
                        print("Runtime error: division by 0");
                    }
                    double result = (x / y);
                    stack_push(&globalStack, &(double){ result }, sizeof(double));
                    break;
            }
        } else if (isPrintStmt(chr)) {
            double val = *(double*)stack_pop(&globalStack);
        } else if (((chr == 10) || (chr == 32))) {
        } else {
            print("Error, unknown character.");
        }
        (i++);
    }
    closef(f);
    closef(f);
}
