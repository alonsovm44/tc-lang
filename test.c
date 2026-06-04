#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)
#include "stdlib/async.h"


#include "stdlib/async.h"
#include "stdlib/io.h"
void worker(int32_t x);
void main(void);

// Wrapper for async function worker
static void worker_wrapper(void *arg) {
    int32_t val = *(int32_t*)arg;
    free(arg);
    worker(val);
}


void worker(int32_t x) {
    printi(x);
}

void main(void) {
    async_init();
    {// Pack arguments for async call
int32_t *arg = malloc(sizeof(int32_t));
*arg = 42;
thread_pool_submit(g_thread_pool, (void(*)(void*))worker_wrapper, arg)
};
    async_shutdown();
}
