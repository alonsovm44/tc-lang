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
#include "C:\Users\diego\.projects\langs\tig\tc\stdlib/async.h"
jmp_buf *tc_get_try_buf(void);
void tc_set_try_buf(jmp_buf *buf);
void tc_throw(const char *error_name);
const char *tc_error_name(void);


typedef struct { char * *ptr; size_t len; } tc_fat_ptr;

#include "C:\Users\diego\.projects\langs\tig\tc\stdlib/io.h"
#include "C:\Users\diego\.projects\langs\tig\tc\stdlib/str.h"
#include "C:\Users\diego\.projects\langs\tig\tc\stdlib/fs.h"
void error_NoMakerfile() {
    print("No Makerfile was found in this directory.");
}

void error_couldNotOpenFile(FILE *file) {
    print("Error, could not open file");
    free(file);
}

char *stringify_file(FILE *f);
Queue tokenize(char *str);
char checkMF(void);
int32_t countTokens(FILE *f);
int32_t main(int argc, char **argv);


char *stringify_file(FILE *f) {
    int32_t size = getFileSize(f);
    char *fcontent = TC_ALLOC(char, (size + 1));
    if ((!fcontent)) {
        
        return "";
    }
    int32_t c = {0};
    int32_t i = 0;
    while (1) {
        (c = filegetc(f));
        if ((c == (-1))) {
            break;
        }
        (fcontent[i] = c);
        (i++);
    }
    (fcontent[i] = 0);
    
    return fcontent;
}

Queue tokenize(char *str) {
    Queue tokensq = queue_create(0);
    int32_t len = slen(str);
    char *word = TC_ALLOC(char, 256);
    int32_t wordlen = 0;
    int32_t i = 0;
    while ((i < len)) {
        char c = str[i];
        if (((((((((((((((c == 58) || (c == 59)) || (c == 61)) || (c == 123)) || (c == 125)) || (c == 60)) || (c == 62)) || (c == 64)) || (c == 35)) || (c == 44)) || (c == 46)) || (c == 42)) || (c == 47)) || (c == 92))) {
            if ((wordlen > 0)) {
                (word[wordlen] = 0);
                char *token = sclone(word);
                queue_push(&tokensq, &(char *){ token }, sizeof(char *));
                (wordlen = 0);
            }
            char *special = TC_ALLOC(char, 2);
            (special[0] = c);
            (special[1] = 0);
            queue_push(&tokensq, &(char *){ special }, sizeof(char *));
            (i++);
        } else if (((((c == 32) || (c == 9)) || (c == 10)) || (c == 13))) {
            if ((wordlen > 0)) {
                (word[wordlen] = 0);
                char *token = sclone(word);
                queue_push(&tokensq, &(char *){ token }, sizeof(char *));
                (wordlen = 0);
            }
            (i++);
        } else {
            (word[wordlen] = c);
            (wordlen++);
            if ((wordlen >= 255)) {
                (word[wordlen] = 0);
                char *token = sclone(word);
                queue_push(&tokensq, &(char *){ token }, sizeof(char *));
                (wordlen = 0);
            }
            (i++);
        }
    }
    if ((wordlen > 0)) {
        (word[wordlen] = 0);
        char *token = sclone(word);
        queue_push(&tokensq, &(char *){ token }, sizeof(char *));
    }
    
    free(word);
    return tokensq;
}

char checkMF(void) {
    if ((fileExists("Makerfile") == 1)) {
        
        return 1;
    }
    error_NoMakerfile();
    tc_throw("NoMakerfile");
}

int32_t countTokens(FILE *f) {
    if ((!f)) {
        error_couldNotOpenFile(f);
        tc_throw("couldNotOpenFile");
    }
    int32_t counter = 0;
    char in_word = 0;
    int32_t c = {0};
    while (1) {
        (c = filegetc(f));
        if ((c == (-1))) {
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
    
    closef(f);
    return counter;
}

int32_t main(int argc, char **argv) {
    tc_fat_ptr args = { .ptr = argv, .len = (size_t)argc };
    {
        jmp_buf _try_buf;
        jmp_buf *_old_buf = tc_get_try_buf();
        tc_set_try_buf(&_try_buf);
        int _try_result = setjmp(_try_buf);
        if (_try_result == 0) {
            checkMF();
        } else {
            if (strcmp(tc_error_name(), "NoMakerfile") == 0) {
                return 1;
            }
            else {
                print("Unknown Error");
                
                return (-99);
            }
        }
        tc_set_try_buf(_old_buf);
    }
    char *first_arg = args.ptr[1];
    FILE *mkf = openf("Makerfile", "r");
    char *mkf_content = stringify_file(mkf);
    Queue tokens = tokenize(mkf_content);
    char *first_token = *(char **)queue_pop(&tokens);
    print(first_token);
    
    closef(mkf);
    return 0;
}
