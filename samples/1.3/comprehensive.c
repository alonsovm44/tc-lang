#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)
#include "runtime_simple.h"


#include "stdlib/io.h"
void async_worker(void);
void increment(int32_t *x);
void test_pin(void);
int32_t main(void);


void async_worker(void) {
    printi(42);
}

void increment(int32_t *x) {
    ((*x) = ((*x) + 1));
}

void test_pin(void) {
    int32_t x = 100;
    /* pin  */
    printi(x);
}

int32_t main(void) {
    int32_t value = 15;
    increment(&value);
    printi(value);
    thread_pool_submit(g_thread_pool, (void(*)(void*))async_worker, NULL);
    test_pin();
    
    return 0;
}
