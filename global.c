#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)


#include "C:\Users\diego\.projects\langs\tig\tc\stdlib/io.h"
int32_t main(void);

float PI = 3.14;
__attribute__((section(".rodata"))) uint8_t *str = "Hello, world!";

int32_t main(void) {
    printf("%d", PI);
    print(str);
    
    return 0;
}
