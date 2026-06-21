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


    #include "raylib.h"


void main(void);


void main(void) {
    InitWindow(800, 600, "Test");
    SetTargetFPS(60);
    while ((!WindowShouldClose())) {
        BeginDrawing();
        EndDrawing();
    }
    CloseWindow();
}
