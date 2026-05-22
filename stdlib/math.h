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

int64_t iabs(int64_t x);
int64_t min(int64_t a, int64_t b);
int64_t max(int64_t a, int64_t b);
int64_t clamp(int64_t x, int64_t lo, int64_t hi);
double fabs64(double x);
double sqrt64(double x);
double pow64(double base, double exp);


int64_t iabs(int64_t x) {
    if ((x < 0)) {
        
        return (0 - x);
    }
    
    return x;
}

int64_t min(int64_t a, int64_t b) {
    if ((a < b)) {
        
        return a;
    }
    
    return b;
}

int64_t max(int64_t a, int64_t b) {
    if ((a > b)) {
        
        return a;
    }
    
    return b;
}

int64_t clamp(int64_t x, int64_t lo, int64_t hi) {
    if ((x < lo)) {
        
        return lo;
    }
    if ((x > hi)) {
        
        return hi;
    }
    
    return x;
}

double fabs64(double x) {
    
    return fabs(x);
}

double sqrt64(double x) {
    
    return sqrt(x);
}

double pow64(double base, double exp) {
    
    return pow(base, exp);
}
