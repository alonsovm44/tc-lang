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


void srand(uint32_t seed);
int32_t rand(void);
int32_t rand_range(int32_t min, int32_t max);
int32_t rand_bool(void);


void srand(uint32_t seed) {
    srand(seed);
    
    return;
}

int32_t rand(void) {
    
    return rand();
}

int32_t rand_range(int32_t min, int32_t max) {
    
    return rand_range(min, max);
}

int32_t rand_bool(void) {
    
    return rand_bool();
}
