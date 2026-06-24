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


#include "C:\tig\stdlib/graphics.h"
void main(void);

Color a = {255, 20, 180, 255};
Color white = {255, 255, 255, 255};
Color my_red = {150, 30, 0, 255};

void main(void) {
    InitWindow(800, 600, "Test");
    SetTargetFPS(60);
    while ((!WindowShouldClose())) {
        BeginDrawing();
        ClearBackground(my_red);
        DrawText("Hello from Tig!", 10, 10, 20, white);
        drawPixel(400, 300, white);
        int32_t i = {0};
        while ((i < 100)) {
            drawPixel((400 + i), (300 + i), white);
            (i++);
        }
        DrawLine(0, 0, 800, 600, a);
        EndDrawing();
    }
    CloseWindow();
}
