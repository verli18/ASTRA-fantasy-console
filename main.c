#include "header.h"

int main(int argc, const char* argv[]) {
  cpu cpuCore;

  if(argc == 0) {
    printf("no program file provided!\n");
    return 1;
  }
  FILE *bin = fopen(argv[1], "rb");
  initConsole(&cpuCore, bin);

    while (!WindowShouldClose()) {
      // Check for key press to advance the cycle
      if (IsKeyPressed(KEY_SPACE)) {
            execCycle(&cpuCore);
      }
 
      BeginDrawing();
        ClearBackground(BLACK);

        drawPipelineStages(&cpuCore);
        drawRegisters(&cpuCore);
      EndDrawing(); 
  }

  CloseWindow();
  return 0;
}

void drawStage(int yOffset, pipelineInstruction instruction, const char* instName) {

    DrawRectangleLines(5, yOffset + 5, 180, 50, DEBUG_TEXT);

    const int margin = 5;
    const int fontSize = 10;
    int textX = 5 + margin;
    int textY = yOffset + margin + 4;

    DrawText(TextFormat("Inst: 0x%08X", instruction.instFetch), textX, textY, 10, DEBUG_TEXT);
    
    DrawRectangleLines(130, yOffset+5 , 55, 12, DEBUG_TEXT);
    DrawText(instName, 132, yOffset+6, 10, DEBUG_TEXT);

    textY += fontSize + 1;
    if((instruction.mnemonic != NULL) && instruction.stage > 1){DrawText( instruction.mnemonic, 80, textY, 10, DEBUG_TEXT);}
    DrawText(TextFormat("r%d/%d/%d",
            instruction.status, instruction.rd, instruction.rs1, instruction.rs2),
            textX, textY, fontSize, DEBUG_TEXT);

    textY += fontSize + 1;
    DrawText(TextFormat("T0:0x%08X  T1:0x%08X",
            instruction.tRegisters[0], instruction.tRegisters[1]),
            textX, textY, fontSize, DEBUG_TEXT);

    // Temporary registers T2 & T3
    textY += fontSize + 1;
    DrawText(TextFormat("T2:0x%08X  T3:0x%08X",
            instruction.tRegisters[2], instruction.tRegisters[3]),
            textX, textY, fontSize, DEBUG_TEXT);
}

void drawPipelineStages(cpu *cpuCore){
  DrawRectangle(0, 0, 190, 280, DEBUG_BLUE);
  const char *stages[5] = {"Fetch", "Decode", "Exec", "Mem", "Write"};
  for(int i = 0; i < 5; i++) {
    drawStage(i*55, cpuCore->pipelineStage[i], stages[i]);
  }
}

void drawRegisters(cpu *cpuCore) {
  DrawRectangle(190, 0, 180, 217, DEBUG_BLUE);
  DrawRectangleLines(190, 5, 175, 207, DEBUG_TEXT);

  DrawText("Registers:", 195, 7, 10, DEBUG_TEXT);
  DrawText(TextFormat("pc = 0x%08X", cpuCore->pc), 265, 7, 10, DEBUG_TEXT);
  for(int i = 0; i < 16; i++){
    DrawText(TextFormat("0x%01X", i), 195, i*12+20, 10, DEBUG_TEXT);
    DrawText(TextFormat("0x%08X", cpuCore->registers[i]), 220, i*12+20, 10, DEBUG_TEXT);
    DrawText(TextFormat("0x%08X", cpuCore->registers[i+16]), 290, i*12+20, 10, DEBUG_TEXT);
  }
}

