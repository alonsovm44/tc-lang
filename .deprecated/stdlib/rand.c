#include <stdlib.h>

int rand_range(int min, int max) {
    return min + (rand() % (max - min + 1));
}

int rand_bool(void) {
    return rand() % 2;
}
