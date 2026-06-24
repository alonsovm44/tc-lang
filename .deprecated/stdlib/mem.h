#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)

void zero(void *ptr, uint64_t n);
void copy(void *dest, void *src, uint64_t n);
int32_t memeq(void *a, void *b, uint64_t n);
void fill(void *ptr, int32_t val, uint64_t n);


void zero(void *ptr, uint64_t n) {
    memset(ptr, 0, n);
    
    return;
}

void copy(void *dest, void *src, uint64_t n) {
    memmove(dest, src, n);
    
    return;
}

int32_t memeq(void *a, void *b, uint64_t n) {
    
    return (memcmp(a, b, n) == 0);
}

void fill(void *ptr, int32_t val, uint64_t n) {
    memset(ptr, val, n);
    
    return;
}
