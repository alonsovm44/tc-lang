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
struct Person {
    char age;
    char *name;
};

void bar(struct Person p);
void foo(struct Person *p);
void main(void);


void bar(struct Person p) {
    (p.age = 21);
    (p.name = "Jane");
}

void foo(struct Person *p) {
    (p->age = 23);
    (p->name = "John");
}

void main(void) {
    struct Person p = {0};
    bar(p);
    foo(&p);
    printi(p.age);
    print(p.name);
}
