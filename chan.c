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
#include "stdlib/snq.h"
#include "stdlib/async.h"
void worker(int32_t id, Queue *out);
void main(void);

// Wrapper for async function worker
struct worker_args {int32_t arg0; Queue *arg1; };
static void worker_wrapper(void *arg) {
    struct worker_args *data = arg;
    worker(data->arg0, data->arg1);
    free(arg);
}


void worker(int32_t id, Queue *out) {
    queue_push(out, &(int32_t){ (id * 2) }, sizeof(int32_t));
}

void main(void) {
    Queue results = queue_create(0);
    int32_t i = 0;
    while ((i < 5)) {
        struct worker_args {int32_t arg0; Queue *arg1; };
struct worker_args *arg = malloc(sizeof(struct worker_args));
arg->arg0 = i;
arg->arg1 = &results;
thread_pool_submit(g_thread_pool, (void(*)(void*))worker_wrapper, arg);;
        (i = (i + 1));
    }
    int32_t j = 0;
    while ((j < 5)) {
        int32_t result = *(int32_t*)queue_pop(&results);
        printi(result);
        (j = (j + 1));
    }
    /* cleanup queue/stack */
    queue_destroy(&results);
}
