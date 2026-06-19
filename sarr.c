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


#include "C:\Users\diego\.projects\langs\tig\tc\stdlib/io.h"
#include "C:\Users\diego\.projects\langs\tig\tc\stdlib/snq.h"
typedef enum {
    LETTER,
    NUMBER,
    SYMBOL
} NodeType;

struct ASTNode {
    NodeType type;
    char *name;
    char *value;
    Queue children;
};

void main(void);


void main(void) {
    struct ASTNode a = {0};
    (a.name = "a");
    (a.value = "1");
    struct ASTNode b = {0};
    (b.name = "b");
    (b.value = "2");
    struct ASTNode c = {0};
    (c.name = "c");
    (c.value = "3");
    queue_push(&a.children, &b, sizeof(struct ASTNode));
    queue_push(&a.children, &c, sizeof(struct ASTNode));
}
