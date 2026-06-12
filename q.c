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
int32_t main(void);


int32_t main(void) {
    Queue q = queue_create(0);
    queue_push(&q, &(int32_t){ 1 }, sizeof(int32_t));
    queue_push(&q, &(int32_t){ 2 }, sizeof(int32_t));
    printi(*(int32_t*)queue_pop(&q));
    printi(*(int32_t*)queue_pop(&q));
    Stack s = stack_create(0);
    stack_push(&s, &(int32_t){ 1 }, sizeof(int32_t));
    stack_push(&s, &(int32_t){ 2 }, sizeof(int32_t));
    printi(*(int32_t*)stack_pop(&s));
    printi(*(int32_t*)stack_pop(&s));
    
    return 0;
}
