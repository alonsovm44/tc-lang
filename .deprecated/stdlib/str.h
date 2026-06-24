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


uint64_t slen(char *s);
int32_t seq(char *a, char *b);
void scpy(char *dest, char *src);
void scat(char *dest, char *src);
int32_t sneq(char *a, char *b, uint64_t n);
char *sfind(char *s, int32_t c);
char *sfindlast(char *s, int32_t c);
char *strhas(char *haystack, char *needle);
int32_t ceq(char a, char b);


uint64_t slen(char *s) {
    
    return strlen(s);
}

int32_t seq(char *a, char *b) {
    
    return (strcmp(a, b) == 0);
}

void scpy(char *dest, char *src) {
    strcpy(dest, src);
    
    return;
}

void scat(char *dest, char *src) {
    strcat(dest, src);
    
    return;
}

int32_t sneq(char *a, char *b, uint64_t n) {
    
    return (strncmp(a, b, n) == 0);
}

char *sfind(char *s, int32_t c) {
    
    return strchr(s, c);
}

char *sfindlast(char *s, int32_t c) {
    
    return strrchr(s, c);
}

char *strhas(char *haystack, char *needle) {
    
    return strstr(haystack, needle);
}

int32_t ceq(char a, char b) {
    
    return (a == b);
}
