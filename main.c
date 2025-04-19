#include "header.h"

int main() {
    cpu cpuCore;
    FILE *bin = fopen("ruledef.bin", "rb");
    initConsole(&cpuCore, bin);

    instFunc instructionArray[16];
    initInstructionArray(instructionArray);

    while (!WindowShouldClose()) {
        // Check for key press to advance the cycle
        if (IsKeyPressed(KEY_SPACE)) {
            execCycle(&cpuCore);
        }
        if(IsKeyPressed(KEY_D)) { // dump dCache with tags and dirty bit
            printf("dCache:\n");
            printf("------------------------------------------------------\n");
            for(int line = 0; line < 256; line++) {
                uint16_t tag = (cpuCore.dCacheTags[line * 2] << 8) | cpuCore.dCacheTags[line * 2 + 1];
                uint8_t dirty = (tag & 0x0004) ? 1 : 0; 
                uint16_t tagVal = (tag & 0x0FFF); // lower 12 bits for tag

                printf("0x%02X | Tag: 0x%03X%s | Data: ", line, tagVal, dirty ? "*" : " ");
                for(int b = 0; b < 16; b++) {
                    printf("%02X ", cpuCore.dCache[line * 16 + b]);
                    if ((b + 1) % 8 == 0 && b != 15) printf(" ");
                }
                printf("\n");
            }
            printf("------------------------------------------------------\n\n");
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