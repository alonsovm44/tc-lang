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


    # include <time.h>


int64_t now(void);
int32_t Clock(void);
int64_t sec_to_ms(int64_t sec);
int64_t sec_to_us(int64_t sec);
double ms_to_sec(int64_t ms);
int32_t sleep(uint32_t ms);


int64_t now(void) {
    
    return time((void *)0);
}

int32_t Clock(void) {
    
    return clock();
}

int64_t sec_to_ms(int64_t sec) {
    
    return (sec * 1000);
}

int64_t sec_to_us(int64_t sec) {
    
    return (sec * 1000000);
}

double ms_to_sec(int64_t ms) {
    
    return (double)(ms / 1000.0);
}

int32_t sleep(uint32_t ms) {
    
    return Sleep(ms);
}
