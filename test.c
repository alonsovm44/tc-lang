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


    #include <stdio.h>


int32_t my_printf(char *fmt, ...);
void main(void);


int32_t my_printf(char *fmt, ...) {
    
    return printf(fmt);
}

void main(void) {
    my_printf("Hello, World!\n");
}
