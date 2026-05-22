#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)

#include "stdlib/io.h"
struct __attribute__((packed)) Point {
    int32_t x;
    int32_t y;
};

int32_t manhattan(struct Point p);
void heap_test(void);
void array_test(void);
void ptr_test(void);
void cast_test(void);
void postfix_test(void);
void pin_test(void);
void scope_test(void);
int32_t main(void);


int32_t manhattan(struct Point p) {
    int32_t ax = p.x;
    int32_t ay = p.y;
    if ((ax < 0)) {
        (ax = (0 - ax));
    }
    if ((ay < 0)) {
        (ay = (0 - ay));
    }
    
    return (ax + ay);
}

void heap_test(void) {
    int32_t *arr = TC_ALLOC(int32_t, 10);
    int32_t i = 0;
    while ((i < 10)) {
        (arr[i] = (i * i));
        (i++);
    }
    printi(arr[9]);
    free(arr);
}

void array_test(void) {
    int32_t nums[5] = {10, 20, 30, 40, 50};
    int32_t i = 0;
    while ((i < 5)) {
        printin(nums[i]);
        printn(" ");
        (i++);
    }
    printn("\n");
}

void ptr_test(void) {
    int32_t x = 42;
    int32_t *p = (&x);
    ((*p) = 99);
    printi(x);
}

void cast_test(void) {
    double pi = 3.14;
    int32_t truncated = ((int32_t)(pi));
    printi(truncated);
}

void postfix_test(void) {
    int32_t x = 5;
    printi((x++));
    printi(x);
    printi((x--));
    printi(x);
}

void pin_test(void) {
    int32_t x = 100;
        printi(x);
}

void scope_test(void) {
    int32_t a = 1;
    {
        int32_t b = 2;
        printi((a + b));
    }
    printi(a);
}

int32_t main(void) {
    print("=== structs ===");
    struct Point pt = {0};
    (pt.x = 3);
    (pt.y = (-4));
    printi(manhattan(pt));
    print("=== heap+defer ===");
    heap_test();
    print("=== arrays ===");
    array_test();
    print("=== raw pointers ===");
    ptr_test();
    print("=== cast ===");
    cast_test();
    print("=== postfix ===");
    postfix_test();
    print("=== pin ===");
    pin_test();
    print("=== scopes ===");
    scope_test();
    
    return 0;
}
