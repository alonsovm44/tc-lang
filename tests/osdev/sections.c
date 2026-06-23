#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)


__attribute__((section(".multiboot"))) volatile void kernel_main(void);

__attribute__((section(".rodata"))) double PI = 3.14159;
volatile 
void kernel_main(void) {
}
