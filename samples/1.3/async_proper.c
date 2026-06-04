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
int32_t main(void);


void async_worker(void) {
    printi(42);
}

int32_t main(void) {
    thread_pool_submit(g_thread_pool, (void(*)(void*))async_worker, NULL);
    
    return 0;
}
