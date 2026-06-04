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
void inc(int32_t *x);
int32_t main(void);


void inc(int32_t *x) {
    ((*x) = ((*x) + 1));
}

int32_t main(void) {
    int32_t x = 15;
    inc(&x);
    printi(x);
    
    return 0;
}
