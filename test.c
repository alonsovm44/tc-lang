#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)
#include "async.h"


#include "stdlib/async.h"
#include "stdlib/io.h"
void inc(int32_t *x);
void async_worker(void);
void main(void);


void inc(int32_t *x) {
    ((*x) = ((*x) + 1));
}

void async_worker(void) {
    printi(42);
}

void main(void) {
    async_init();
    int32_t x = 15;
    inc(&x);
    printi(x);
    thread_pool_submit(g_thread_pool, (void(*)(void*))async_worker, NULL);
    async_shutdown();
}
