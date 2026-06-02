#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)


    #include "raylib.h"


void main(void);


void main(void) {
    int32_t screenWidth = 800;
    int32_t screenHeight = 450;
    

        InitWindow(screenWidth, screenHeight, "TightC + Raylib Demo");
        SetTargetFPS(60);
        
        // Ball position and velocity
        float ballX = screenWidth / 2.0f;
        float ballY = screenHeight / 2.0f;
        float velX = 5.0f;
        float velY = 4.0f;
        float ballRadius = 20.0f;
    
    

        while (1) {
            // Update ball position
            ballX += velX;
            ballY += velY;
            
            // Bounce off walls
            if (ballX >= screenWidth - ballRadius || ballX <= ballRadius) {
                velX *= -1.0f;
            }
            if (ballY >= screenHeight - ballRadius || ballY <= ballRadius) {
                velY *= -1.0f;
            }
            
            // Draw
            BeginDrawing();
            ClearBackground(RAYWHITE);
            
            DrawCircle((int)ballX, (int)ballY, ballRadius, RED);
            DrawText("TightC + Raylib = Awesome!", 10, 10, 20, DARKGRAY);
            DrawFPS(screenWidth - 100, 10);
            
            EndDrawing();
            
            if (WindowShouldClose()) break;
        }
        
        CloseWindow();
    
}
