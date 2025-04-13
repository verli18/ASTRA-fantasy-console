#include "raylib.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "CPU.c"

int main() {
    struct CPU cpuCore;
    FILE *bin = fopen("bin/test.bin", "rb");
    initConsole(&cpuCore, bin);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        EndDrawing();
    }


    CloseWindow();
    return 0;
}