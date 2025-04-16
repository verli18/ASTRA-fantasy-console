#include "raylib.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "CPU.c"

int main() {
    struct CPU cpuCore;
    FILE *bin = fopen("test.bin", "rb");
    initConsole(&cpuCore, bin);

    while (!WindowShouldClose()) {
        // Check for key press to advance the cycle
        if (IsKeyPressed(KEY_SPACE)) {
            execCycle(&cpuCore);
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);

        EndDrawing();
    }


    CloseWindow();
    return 0;
}