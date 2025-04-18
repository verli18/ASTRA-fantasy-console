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
        if(IsKeyPressed(KEY_D)) { //dump dCache
            printf("dCache:\n0x00000000: ");
            for(int i = 0; i < 4096; i++) {
                printf("%02X ", cpuCore.dCache[i]);
                if((i + 1) % 16 == 0) {
                    printf("\n0x%08X: ", i);
                }
            }
        }

        if(IsKeyPressed(KEY_R)) { //registers
            printf("Registers:\n");
            for(int i = 0; i < 16; i++) {
                printf("r%02X: 0x%08X | ", i, cpuCore.registers[i]);
                printf("r%02X: 0x%08X\n", i + 16, cpuCore.registers[i + 16]);
            }
        }
        
        BeginDrawing();
            ClearBackground(RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}