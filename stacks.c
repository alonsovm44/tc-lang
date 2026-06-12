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
void print_stack(Stack s);
void main(void);


void print_stack(Stack s) {
    while ((!stack_isEmpty(&s))) {
        printi(*(int32_t*)stack_pop(&s));
    }
}

void main(void) {
    Stack s = stack_create(0);
    stack_push(&s, &(int32_t){ 1 }, sizeof(int32_t));
    stack_push(&s, &(int32_t){ 2 }, sizeof(int32_t));
    stack_push(&s, &(int32_t){ 3 }, sizeof(int32_t));
    print_stack(s);
    /* cleanup queue/stack */
    stack_destroy(&s);
}
