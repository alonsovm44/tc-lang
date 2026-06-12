#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)


#include "stdlib/fs.h"
#include "stdlib/io.h"
void main(void);


void main(void) {
    FILE *f = openf("test.txt", "r");
    if ((f == 0)) {
        println("Failed to open file");
        
        return;
    }
    fclose(f);
}
