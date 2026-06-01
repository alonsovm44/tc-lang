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
void main(void);


void main(void) {
    print("Testing Bitwise Operations\n");
    print("==========================\n\n");
    uint8_t a = 202;
    uint8_t b = 182;
    print("a = 0b11001010 (202)\n");
    print("b = 0b10110110 (182)\n\n");
    uint8_t result = (a & b);
    print("a & b (AND):\n");
    print("  11001010\n");
    print("& 10110110\n");
    print("  --------\n");
    print("  10000010 (");
    printi(result);
    print(")\n\n");
    (result = (a | b));
    print("a | b (OR):\n");
    print("  11001010\n");
    print("| 10110110\n");
    print("  --------\n");
    print("  11111110 (");
    printi(result);
    print(")\n\n");
    (result = (a ^ b));
    print("a ^ b (XOR):\n");
    print("  11001010\n");
    print("^ 10110110\n");
    print("  --------\n");
    print("  01111100 (");
    printi(result);
    print(")\n\n");
    (result = (!a));
    print("!a (NOT):\n");
    print("  11001010\n");
    print("! --------\n");
    print("  00110101 (");
    printi(result);
    print(")\n\n");
    (result = (a << 2));
    print("a << 2 (Shift Left):\n");
    print("  11001010 << 2\n");
    print("  00101000 (");
    printi(result);
    print(")\n\n");
    (result = (a >> 2));
    print("a >> 2 (Shift Right):\n");
    print("  11001010 >> 2\n");
    print("  00110010 (");
    printi(result);
    print(")\n\n");
    print("Bitwise with Assignment:\n");
    uint8_t x = 15;
    print("x = 0b00001111 (15)\n");
    (x = (x << 1));
    print("x = x << 1: ");
    printi(x);
    print("\n");
    (x = (x >> 1));
    print("x = x >> 1: ");
    printi(x);
    print("\n");
    (x = (x | 240));
    print("x = x | 0b11110000: ");
    printi(x);
    print("\n");
    (x = (x & 170));
    print("x = x & 0b10101010: ");
    printi(x);
    print("\n");
    (x = (x ^ 255));
    print("x = x ^ 0b11111111: ");
    printi(x);
    print("\n\n");
    print("Practical Examples:\n");
    uint8_t flags = 0;
    print("flags = 0b00000000\n");
    (flags = (flags | 4));
    print("Set bit 2: ");
    printi(flags);
    print("\n");
    (flags = (flags & 251));
    print("Clear bit 2: ");
    printi(flags);
    print("\n");
    (flags = (flags ^ 4));
    print("Toggle bit 2: ");
    printi(flags);
    print("\n");
    uint8_t mask = 4;
    if (((flags & mask) != 0)) {
        print("Bit 2 is set\n");
    } else {
        print("Bit 2 is not set\n");
    }
    print("\nBitwise operations test completed!\n");
}
