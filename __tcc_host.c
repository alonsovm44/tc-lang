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


// Hot function dispatch table
int32_t(*hot_add)(int32_t, int32_t);

#include <windows.h>
static HMODULE hot_lib_handle = NULL;

static void load_hot_functions(void) {
    hot_lib_handle = LoadLibraryA("hotlib.dll");
    if (!hot_lib_handle) { fprintf(stderr, "Failed to load hotlib.dll\n"); return; }
    hot_add = (void*)GetProcAddress(hot_lib_handle, "add");
    if (!hot_add) fprintf(stderr, "Failed to load function add\n");
}

void main(void);

int32_t add(int32_t x, int32_t y) {
    return hot_add(x, y);
}


void main(void) {
    load_hot_functions();
    int32_t result = add(3, 4);
    printi(result);
}
