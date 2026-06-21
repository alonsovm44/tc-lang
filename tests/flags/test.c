#include "raylib.h"

int main(void) {
    InitWindow(800, 600, "Test C");
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Hello from C!", 10, 10, 20, WHITE);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}