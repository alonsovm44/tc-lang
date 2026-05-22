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
#include "stdlib/str.h"
#include "stdlib/math.h"
#include "stdlib/mem.h"
#include "stdlib/conv.h"
int32_t main(void);


int32_t main(void) {
    char *hello = "hello";
    char *world = "world";
    printi(slen(hello));
    if (seq(hello, "hello")) {
        print("strings equal");
    }
    printi(iabs((0 - 42)));
    printi(min(10, 3));
    printi(max(10, 3));
    printi(clamp(50, 0, 20));
    int64_t n = stoi("12345");
    printi(n);
    char buf[32] = {0};
    itos(n, (&buf[0]), 32);
    print((&buf[0]));
    int32_t a[4] = {1, 2, 3, 4};
    int32_t b[4] = {0};
    copy((&b[0]), (&a[0]), 16);
    printi(b[2]);
    
    return 0;
}
