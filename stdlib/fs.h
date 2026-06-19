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


FILE *openf(char *file, char *mode);
int32_t closef(FILE *f);
int32_t filegetc(FILE *stream);
int32_t fputchr(int32_t c, FILE *stream);
int32_t fputstr(char *s, FILE *stream);
int32_t iseof(FILE *stream);
uint64_t readf(void *ptr, uint64_t size, uint64_t count, void *file);
uint64_t writef(void *ptr, uint64_t size, uint64_t count, void *file);
int32_t seekf(FILE *file, int64_t offset, int32_t whence);
int64_t tellf(FILE *file);
int32_t errorf(FILE *file);
char fileExists(char *path);
int32_t getFileSize(FILE *f);


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

int32_t iseof(FILE *stream) {
    
    return feof(stream);
}

uint64_t readf(void *ptr, uint64_t size, uint64_t count, void *file) {
    
    return fread(ptr, size, count, file);
}

uint64_t writef(void *ptr, uint64_t size, uint64_t count, void *file) {
    
    return fwrite(ptr, size, count, file);
}

int32_t seekf(FILE *file, int64_t offset, int32_t whence) {
    
    return fseek(file, offset, whence);
}

int64_t tellf(FILE *file) {
    
    return ftell(file);
}

int32_t errorf(FILE *file) {
    
    return ferror(file);
}

char fileExists(char *path) {
    FILE *f = openf(path, "r");
    if ((!f)) {
        closef(f);
        
        return 0;
    }
    closef(f);
    
    return 1;
}

int32_t getFileSize(FILE *f) {
    int32_t i = 0;
    int32_t c = 0;
    int32_t start_pos = tellf(f);
    while (1) {
        if ((c == (-1))) {
            break;
        }
        (c = filegetc(f));
        (i++);
    }
    seekf(f, start_pos, 0);
    
    return i;
}
