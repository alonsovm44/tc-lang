#include <stdio.h>
#include <stdlib.h>

void assert(int cond, const char *msg) {
    if (!cond) {
        fprintf(stderr, "assertion failed: %s\n", msg);
        fflush(stderr);
        abort();
    }
}

void panic(const char *msg) {
    fprintf(stderr, "panic: %s\n", msg);
    fflush(stderr);
    abort();
}
