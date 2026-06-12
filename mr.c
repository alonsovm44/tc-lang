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
#include "stdlib/snq.h"
void multiple_return_fn(int32_t x, int32_t y, Queue q);
int32_t main(void);


void multiple_return_fn(int32_t x, int32_t y, Queue q) {
    queue_push(&q, &(int32_t){ (x + y) }, sizeof(int32_t));
    queue_push(&q, &(int32_t){ (x * y) }, sizeof(int32_t));
}

int32_t main(void) {
    int32_t a = 10;
    int32_t b = 20;
    Queue q = queue_create(0);
    multiple_return_fn(a, b, q);
    printi(*(int32_t*)queue_pop(&q));
    printi(*(int32_t*)queue_pop(&q));
    /* cleanup queue/stack */
    queue_destroy(&q);
}
