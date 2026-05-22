#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)

typedef struct { int32_t * *ptr; size_t len; } tc_fat_ptr;
typedef struct { int32_t *ptr; size_t len; } tc_fat_i32;

#include "stdlib/io.h"
void main(void);


void main(void) {
    int32_t val = 42;
    int32_t *p = (&val);
    int32_t **pp = (&p);
    print("->-> pointer to pointer:");
    printi((*(*pp)));
    int32_t a = 10;
    int32_t b = 20;
    int32_t c = 30;
    int32_t *ptrs[3] = {(&a), (&b), (&c)};
    tc_fat_ptr fp = { .ptr = ptrs, .len = TC_LENOF(ptrs) };
    print("=>-> fat pointer of raw pointers:");
    printi(fp.len);
    printi((*fp.ptr[0]));
    printi((*fp.ptr[1]));
    printi((*fp.ptr[2]));
    int32_t arr[4] = {100, 200, 300, 400};
    tc_fat_i32 slice = { .ptr = arr, .len = TC_LENOF(arr) };
    tc_fat_i32 *pslice = (&slice);
    print("->=> raw pointer to fat pointer:");
    printi((*pslice).len);
    printi((*pslice).ptr[0]);
    printi((*pslice).ptr[3]);
}
