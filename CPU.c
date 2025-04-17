#include "header.h"

void initConsole(struct CPU *cpuCore, FILE *bin){
    InitWindow(640, 480, "ASTRA");
    //TODO: Implement cache logic
    cpuCore->iCache = calloc(4096, sizeof(uint8_t));
    cpuCore->dCache = calloc(4096, sizeof(uint8_t));
    cpuCore->pc = cpuStartVec;
    for(int i = 0; i < 32; i++) {
        cpuCore->registers[i] = i;
    }

    fread(cpuCore->iCache, 1, 4096, bin);
}

uint32_t fetchWord(uint8_t *pointer, uint32_t address) {
    return (pointer[address] << 24) | 
    (pointer[address + 1] << 16) | 
    (pointer[address + 2] << 8) | 
    pointer[address + 3];

}

pipelineInstruction instructionFetch( struct CPU *cpuCore) {
    pipelineInstruction fetch;
    fetch.instFetch = fetchWord(cpuCore->iCache, cpuCore->pc);
    cpuCore->pc += 4;
    fetch.status = 1;
    return fetch;
}

pipelineInstruction instructionDecode(struct CPU *cpuCore, pipelineInstruction decode) {
    int instruction = decode.instFetch;

    if ((instruction & 0x80000000) == 0) { //register operation
        decode.tRegisters[0] = cpuCore->registers[(instruction & 0x01F00000) >> 20];
        decode.tRegisters[1] = cpuCore->registers[(instruction & 0x00007C00) >> 10];
        decode.tRegisters[2] = cpuCore->registers[(instruction & 0x0000001F)];
    }
    return decode;
}

pipelineInstruction instructionExecute(struct CPU *cpuCore, pipelineInstruction execute) {

    switch((execute.instFetch >> 24)& 0xFE) { //operation type
        case 0x00: //LOADW
         break;
        case 0x02: //STOREW
         break;
        case 0x04: //LODB
         break;
        case 0x06: //STRB
         break;
        case 0x08: //MOV
         break;
        case 0x0A: //SWAP
         break;
        case 0x0C: //LFR
         break;
        case 0x0E: //SFR
         break;
        case 0x10: //WCL
         break;
        case 0x12: //LLI
         break;
        case 0x14: //LUI
         break;
        case 0x16: //LSI
         break;
        case 0x18: //LDO
         break;
        case 0x1A: //STO
         break;         
    }
    return execute;
}


int execCycle(struct CPU *cpuCore) {

    cpuCore->pipelineStage[1] = instructionDecode(cpuCore, cpuCore->pipelineStage[0]);
    cpuCore->pipelineStage[0] = instructionFetch(cpuCore);

    printf("new cycle-----------------------------\n");
    printf("PC: 0x%08X\n\n", cpuCore->pc);
    printPipelineStage(cpuCore->pipelineStage[0], "Fetch:");
    printPipelineStage(cpuCore->pipelineStage[1], "Decode:");
    printf("--------------------------------------\n\n");

    return 0;
}

void printPipelineStage(struct pipelineInstruction stage, const char *stageName) {
    printf("%s\n", stageName);
    printf("Instruction: 0x%08X\n", stage.instFetch);
    printf("Status: 0x%02X\n", stage.status);
    printf("T0: 0x%08X  |  ", stage.tRegisters[0]);
    printf("T1: 0x%08X\n", stage.tRegisters[1]);
    printf("T2: 0x%08X  |  ", stage.tRegisters[2]);
    printf("T3: 0x%08X\n\n", stage.tRegisters[3]);
}