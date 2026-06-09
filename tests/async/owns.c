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
#include "stdlib/str.h"
#include "stdlib/async.h"
void worker(char *str, Queue *q);
int32_t main(void);

// Wrapper for async function worker
struct worker_args {char *arg0; Queue *arg1; };
static void worker_wrapper(void *arg) {
    struct worker_args *data = arg;
    worker(data->arg0, data->arg1);
    free(arg);
}


void worker(char *str, Queue *q) {
    if (strhas(str, "wololo")) {
        queue_push(q, &(char){ 1 }, sizeof(char));
    } else {
        queue_push(q, &(char){ 0 }, sizeof(char));
    }
}

int32_t main(void) {
    char *string = "wahahawololo";
    Queue q = queue_create(0);
    struct worker_args {char *arg0; Queue *arg1; };
struct worker_args *arg = malloc(sizeof(struct worker_args));
arg->arg0 = string;
arg->arg1 = &q;
thread_pool_submit(g_thread_pool, (void(*)(void*))worker_wrapper, arg);;
    char result = *(char*)queue_pop(&q);
    if ((result == 1)) {
        print("string has wololo");
    } else {
        print("string has no wololo");
    }
    
    return 0;
}
