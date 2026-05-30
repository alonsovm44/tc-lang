#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)

#include "stdlib/io.h"
int32_t isNextCharBlank(FILE *f);
void main(void);


int32_t isNextCharBlank(FILE *f) {
    int32_t c = fgetc(f);
    if ((c != "EOF")) {
        ungetc(c, f);
        
        return (c == " ");
    }
    
    return c;
}

void main(void) {
    FILE *file = fopen("test.txt", "r");
    if ((!file)) {
        print("Error, could not open file");
        
        return;
    }
    int32_t count = 0;
    while ((!eof(file))) {
        if (isNextCharBlank(file)) {
        } else {
            fgetc(file);
            (count++);
        }
    }
    print("Word count: ");
    printi(count);
    print("\n");
}
