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
struct TestStruct {
    int32_t a;
    int64_t b;
    float c;
};

void main(void);


void main(void) {
    print("Testing sizeof functionality\n");
    int32_t i32_size = sizeof(int32_t);
    int32_t i64_size = sizeof(int64_t);
    int32_t f32_size = sizeof(float);
    int32_t f64_size = sizeof(double);
    print("i32 size: ");
    printi(i32_size);
    print("\n");
    print("i64 size: ");
    printi(i64_size);
    print("\n");
    print("f32 size: ");
    printi(f32_size);
    print("\n");
    print("f64 size: ");
    printi(f64_size);
    print("\n");
    int32_t struct_size = sizeof(struct TestStruct);
    print("TestStruct size: ");
    printi(struct_size);
    print("\n");
    int32_t ptr_size = sizeof(int32_t *);
    print("Pointer size: ");
    printi(ptr_size);
    print("\n");
    print("sizeof test completed!\n");
}
