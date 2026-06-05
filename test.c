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


#include "stdlib/io.h"
#include "stdlib/async.h"
void foo(int32_t *x);
int32_t main(void);

// Wrapper for async function foo
static void foo_wrapper(void *arg) {
    int32_t val = *(int32_t*)arg;
    free(arg);
    foo(val);
}


void foo(int32_t *x) {
    ((*x) = ((*x) + 1));
}

int32_t main(void) {
    int32_t x = 10;
    {// Pack arguments for async call
int32_t *arg = malloc(sizeof(int32_t));
*arg = (&refx);
thread_pool_submit(g_thread_pool, (void(*)(void*))foo_wrapper, arg);
};
    printi(x);
}
