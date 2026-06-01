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
    print("Testing allocation and defer\n");
    int32_t size = 10;
    int32_t *ptr = TC_ALLOC(int32_t, size);
    print("Allocated array of size: ");
    printi(size);
    print("\n");
    int32_t i = 0;
    while (1) {
        if ((i >= size)) {
            break;
        }
        (ptr[i] = (i * 2));
        (i = (i + 1));
    }
    print("Array values: ");
    (i = 0);
    while (1) {
        if ((i >= size)) {
            break;
        }
        printi(ptr[i]);
        print(" ");
        (i = (i + 1));
    }
    print("\n");
    print("\nTesting multiple defers:\n");
    int32_t *ptr2 = TC_ALLOC(int32_t, 5);
    int32_t *ptr3 = TC_ALLOC(int32_t, 3);
    print("Allocations done, defers will execute in reverse order\n");
    print("\nAllocation and defer test completed!\n");
    print("First defer executed\n");
    free(ptr3);
    print("Second defer executed\n");
    free(ptr2);
    print("Cleaning up allocated memory\n");
    free(ptr);
}
