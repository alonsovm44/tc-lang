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
int32_t main(void);


int32_t main(void) {
    char *file = "test.txt";
    FILE *f = open_file(file, "r");
    if ((!f)) {
        print("Error, could not open file");
        
        return 1;
    }
    int32_t counter = 0;
    char in_word = 0;
    int32_t char_count = 0;
    while (1) {
        int32_t c = fgetc(f);
        printn("Read char: ");
        printi(c);
        (char_count = (char_count + 1));
        if ((char_count > 100)) {
            printn("Breaking after 100 chars to avoid infinite loop");
            break;
        }
        if ((c == (-1))) {
            printn("EOF reached!");
            break;
        }
        if ((c == 32)) {
            (in_word = 0);
        } else {
            if ((in_word == 0)) {
                (counter++);
                (in_word = 1);
            }
        }
    }
    fclose(f);
    printn("Words: ");
    printi(counter);
    
    return 0;
}
