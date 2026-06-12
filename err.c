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
void error_zeroDivision(int32_t a, int32_t b) {
    printn("Zero division");
    printi(a);
    printn(" / ");
    printi(b);
    printn("");
}

int32_t main(void);


int32_t main(void) {
    int32_t a = 10;
    int32_t b = 0;
    {
        if ((b == 0)) {
            /* throw zeroDivision(a, b) */
        }
        printn("No error");
        
        return 0;
    }
/* catch block */
}
