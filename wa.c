#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)


void consume(int32_t *p);
int32_t main(void);


void consume(int32_t *p) {
}

int32_t main(void) {
    int32_t *x = alloc(int32_t);
    consume((&x));
    
    return 0;
}
