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
int32_t main(void);


int32_t main(void) {
    printn("Opening file...");
    FILE *f = open_file("test.txt", "rb");
    if ((!f)) {
        print("Error: could not open file");
        
        return 1;
    }
    printn("File opened, reading first char...");
    int32_t c = fgetc(f);
    printn("First char: ");
    printi(c);
    fclose(f);
    printn("Done");
    
    return 0;
}
