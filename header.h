#include "raylib.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"

#define cpuStartVec 0x00000000

typedef struct pipelineInstruction {
    uint32_t status;
    uint32_t instFetch;
    uint32_t tRegisters[4];
} pipelineInstruction;

struct CPU {
    uint32_t registers[32];
    pipelineInstruction pipelineStage[5];
    uint32_t pc;
    uint8_t *iCache;
    uint8_t *iCacheTags;
    uint8_t *dCache;
    uint8_t *dCacheTags;
};
//helper functions
uint32_t fetchWord(uint8_t *pointer, uint32_t address);

//other functions
void initConsole(struct CPU *cpuCore, FILE *bin);

//execution related functions
pipelineInstruction instructionFetch( struct CPU *cpuCore);
pipelineInstruction instructionDecode(struct CPU *cpuCore, pipelineInstruction decode);
pipelineInstruction instructionExecute(struct CPU *cpuCore, pipelineInstruction execute);
int execCycle(struct CPU *cpuCore);

//debug functions
void printPipelineStage(pipelineInstruction stage, const char *stageName);
