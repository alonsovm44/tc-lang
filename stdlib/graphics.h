#pragma once
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


void targetFPS(int32_t fps);
char windowShouldClose(void);
void makeWindow(int32_t width, int32_t height, char *title);
void clearbg(Color color);
void drawPixel(int32_t x, int32_t y, Color color);
void drawText(char *text, int32_t x, int32_t y, int32_t fontSize, Color color);
void drawCircle(int32_t x, int32_t y, float radius, Color color);
void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, Color color);

Color Raywhite = RAYWHITE;
Color Black = BLACK;

void targetFPS(int32_t fps) {
    SetTargetFPS(fps);
}

char windowShouldClose(void) {
    
    return WindowShouldClose();
}

void makeWindow(int32_t width, int32_t height, char *title) {
    InitWindow(width, height, title);
}

void clearbg(Color color) {
    ClearBackground(color);
}

void drawPixel(int32_t x, int32_t y, Color color) {
    DrawRectangle(x, y, 1, 1, color);
}

void drawText(char *text, int32_t x, int32_t y, int32_t fontSize, Color color) {
    DrawText(text, x, y, fontSize, color);
}

void drawCircle(int32_t x, int32_t y, float radius, Color color) {
    DrawCircle(x, y, radius, color);
}

void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, Color color) {
    DrawRectangle(x, y, w, h, color);
}
