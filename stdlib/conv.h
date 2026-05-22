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

int64_t stoi(char *s);
int64_t stoib(char *s, int32_t base);
double stof(char *s);
int32_t itos(int64_t n, char *buf, uint64_t bufsize);
int32_t ftos(double n, char *buf, uint64_t bufsize);


int64_t stoi(char *s) {
    
    return strtol(s, ((char **)(0)), 10);
}

int64_t stoib(char *s, int32_t base) {
    
    return strtol(s, ((char **)(0)), base);
}

double stof(char *s) {
    
    return strtod(s, ((char **)(0)));
}

int32_t itos(int64_t n, char *buf, uint64_t bufsize) {
    
    return snprintf(buf, bufsize, "%lld", n);
}

int32_t ftos(double n, char *buf, uint64_t bufsize) {
    
    return snprintf(buf, bufsize, "%g", n);
}
