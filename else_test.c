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
void main(void);


void main(void) {
    int32_t score = 85;
    if ((score >= 90)) {
        print("A\n");
    } else if ((score >= 80)) {
        print("B\n");
    } else if ((score >= 70)) {
        print("C\n");
    } else if ((score >= 60)) {
        print("D\n");
    } else {
        print("F\n");
    }
}
