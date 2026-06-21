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


void assert(int32_t cond, char *msg);
void panic(char *msg);


void assert(int32_t cond, char *msg) {
    assert(cond, msg);
    
    return;
}

void panic(char *msg) {
    panic(msg);
    
    return;
}
