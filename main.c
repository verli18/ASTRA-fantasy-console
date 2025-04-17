#include "header.h"

int main() {
    struct CPU cpuCore;
    FILE *bin = fopen("ruledef.bin", "rb");
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