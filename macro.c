#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)


    static void printi(int32_t x) { printf("%d\n", x); }


void main(void);


void main(void) {
    int32_t x = 10;
    float y = 3.14;
    uint8_t c = 65;
    int32_t doubled = 5 * 2;
    int32_t squared = 4 * 4;
    int32_t sum = 3 + 7;
    int32_t a = 1;
    int32_t b = 2;
    int32_t temp = a;
    (a = b);
    (b = temp);
    printi(doubled);
    printi(squared);
    printi(sum);
    printi(a);
    printi(b);
}
