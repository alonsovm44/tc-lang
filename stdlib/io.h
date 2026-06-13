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


void print(char *s);
void printi(int64_t n);
void printn(char *s);
int32_t readc(void);
int32_t putchr(int32_t c);


void print(char *s) {
    printf("%s", s);
}

void printi(int64_t n) {
    printf("%ld", n);
}

void printn(char *s) {
    printf("%s", s);
    putchar(10);
}

int32_t readc(void) {
    
    return getchar();
}

int32_t putchr(int32_t c) {
    
    return putchar(c);
}
