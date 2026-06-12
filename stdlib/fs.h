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


FILE *openf(char *file, char *mode);
int32_t closef(FILE *f);
int32_t filegetc(FILE *stream);
int32_t fputchr(int32_t c, FILE *stream);
int32_t fputstr(char *s, FILE *stream);
int32_t eof(FILE *stream);


FILE *openf(char *file, char *mode) {
    
    return fopen(file, mode);
}

int32_t closef(FILE *f) {
    
    return fclose(f);
}

int32_t filegetc(FILE *stream) {
    
    return fgetc(stream);
}

int32_t fputchr(int32_t c, FILE *stream) {
    
    return fputc(c, stream);
}

int32_t fputstr(char *s, FILE *stream) {
    
    return fputs(s, stream);
}

int32_t eof(FILE *stream) {
    
    return feof(stream);
}
