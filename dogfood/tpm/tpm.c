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


typedef struct { char * *ptr; size_t len; } tc_fat_ptr;

#include "C:\tig\stdlib/io.h"
#include "C:\tig\stdlib/fs.h"
#include "C:\tig\stdlib/os.h"
#include "C:\tig\stdlib/str.h"
void error_E001() {
    print("Error while creating project: ");
    print("\n");
}

int32_t init(void);
int32_t main(int argc, char **argv);


int32_t init(void) {
    char pnameBuff[256] = {0};
    char buildsystemBuff[256] = {0};
    printf("Project name: ");
    scan("%s", pnameBuff);
    if ((pnameBuff[0] == 0)) {
        printf("Project name cannot be empty\n");
        
        return 1;
    }
    printf("Build system (make/cmake): ");
    scan("%s", buildsystemBuff);
    if ((buildsystemBuff[0] == 0)) {
        printf("Build system cannot be empty\n");
        
        return 1;
    }
    char mkdircmd[512] = {};
    char *pname = {0};
    int32_t i = {0};
    while ((i < pnameBuff.len)) {
        (pname[i] = pnameBuff[i]);
        (i++);
    }
    fprints(mkdircmd, "mkdir %s", pname);
    os(mkdircmd);
    
    return 0;
}

int32_t main(int argc, char **argv) {
    tc_fat_ptr args = { .ptr = argv, .len = (size_t)argc };
    if ((args.len < 2)) {
        printf("Usage: tpm <command>\n");
        
        return 1;
    }
    char *cmd = args.ptr[1];
    if ((strcmp(cmd, "init") == 0)) {
        init();
        
        return 0;
    }
    
    return 0;
}
