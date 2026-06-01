#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)


void main(void);


void main(void) {
    int32_t a = 1;
    int32_t b = 2;
    int32_t temp = a;
    (a = b);
    (b = temp);
    printi(a);
    printi(b);
}
