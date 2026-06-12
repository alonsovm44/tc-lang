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
void func(Queue q);
int32_t main(void);


void func(Queue q) {
    if ((queue_size(&q) == 0)) {
        print("Queue is empty\n");
    } else {
        print("Queue is not empty\n");
    }
}

int32_t main(void) {
    Queue q = queue_create(3);
    queue_push(&q, &(int32_t){ 0 }, sizeof(int32_t));
    queue_push(&q, &(int32_t){ 1 }, sizeof(int32_t));
    queue_push(&q, &(int32_t){ 2 }, sizeof(int32_t));
    func(q);
    
    return 0;
}
