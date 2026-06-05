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
void main(void);


void main(void) {
    Queue q = {0, 1, 2};
    queue_push(&q, &(int32_t){ 3 }, sizeof(int32_t));
    int32_t x = *(int32_t*)queue_pop(&q);
    int32_t y = *(int32_t*)queue_pop(&q);
    printi(x);
    printi(y);
}
