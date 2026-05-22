#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define lenof(x) (sizeof(x) / sizeof((x)[0]))

#include "stdlib/io.h"
void loops(void);
void main(void);


void loops(void) {
    int32_t i = 10;
    while ((i > 0)) {
        (i -= 1);
        printi(i);
    }
}

void main(void) {
    loops();
}
