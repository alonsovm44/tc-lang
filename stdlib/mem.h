#pragma once
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


void zero(void *ptr, uint64_t n);
void movemem(void *dest, void *src, uint64_t n);
int32_t cmpmem(void *a, void *b, uint64_t n);
void setmem(void *ptr, int32_t val, uint64_t n);


void zero(void *ptr, uint64_t n) {
    memset(ptr, 0, n);
    
    return;
}

void movemem(void *dest, void *src, uint64_t n) {
    memmove(dest, src, n);
    
    return;
}

int32_t cmpmem(void *a, void *b, uint64_t n) {
    
    return (memcmp(a, b, n) == 0);
}

void setmem(void *ptr, int32_t val, uint64_t n) {
    memset(ptr, val, n);
    
    return;
}
