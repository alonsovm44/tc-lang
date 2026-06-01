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
    print("Testing simple parametric macro\n");
    print("=================================\n\n");
    int32_t result = "\n";
    print("twice(5) = ");
    printi(result);
    print("\n");
    print("Parametric macro test completed!\n");
}
