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


int32_t write_file(char *s, FILE *stream);
int32_t fputc(int32_t c, FILE *stream);
FILE *open_file(char *file, char *mode);
int32_t fclose(FILE *f);
int32_t fgetc(FILE *stream);
int32_t fprintpf(FILE *stream, char *fmt, ...);
void print(char *s);
void printflt(double n);
void printn(char *s);
void printi(int64_t n);
void printin(int64_t n);
int32_t readc(void);
int32_t unreadc(int32_t c, FILE *stream);
int32_t eof(FILE *stream);


int32_t write_file(char *s, FILE *stream) {
    
    return fputs(s, stream);
}

int32_t fputc(int32_t c, FILE *stream) {
    
    return fputc(c, stream);
}

FILE *open_file(char *file, char *mode) {
    
    return fopen(file, mode);
}

int32_t fclose(FILE *f) {
    
    return fclose(f);
}

int32_t fgetc(FILE *stream) {
    
    return fgetc(stream);
}

int32_t fprintpf(FILE *stream, char *fmt, ...) {
    
    return fprintf(stream, fmt);
}

void print(char *s) {
    printf(s);
    putchar(10);
    
    return;
}

void printflt(double n) {
    printf("%f", n);
    putchar(10);
    
    return;
}

void printn(char *s) {
    printf(s);
    
    return;
}

void printi(int64_t n) {
    printf("%lld\n", n);
    
    return;
}

void printin(int64_t n) {
    printf("%lld", n);
    
    return;
}

int32_t readc(void) {
    
    return getchar();
}

int32_t unreadc(int32_t c, FILE *stream) {
    
    return ungetc(c, stream);
}

int32_t eof(FILE *stream) {
    
    return feof(stream);
}
