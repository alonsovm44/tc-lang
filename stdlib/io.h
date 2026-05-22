#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define lenof(x) (sizeof(x) / sizeof((x)[0]))

void print(char *s);
void printn(char *s);
void printi(int64_t n);
void printin(int64_t n);
int64_t readi(void);
int32_t readc(void);


void print(char *s) {
    printf(s);
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

int64_t readi(void) {
    int64_t n = {0};
    scanf("%lld", (&n));
    
    return n;
}

int32_t readc(void) {
    
    return getchar();
}
