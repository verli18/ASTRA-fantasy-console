#include "header.h"

void initConsole(struct CPU *cpuCore, FILE *bin){
    InitWindow(640, 480, "ASTRA");
    //TODO: Implement cache logic
    cpuCore->iCache = calloc(4096, sizeof(uint8_t));
    cpuCore->dCache = calloc(4096, sizeof(uint8_t));
    cpuCore->pc = cpuStartVec;
    for(int i = 0; i < 32; i++) {
        cpuCore->registers[i] = 0;
    }

    fread(cpuCore->iCache, 1, 4096, bin);
}

//these will need to be expanded to work on cache later
void writeWord(uint8_t *pointer, uint32_t address, uint32_t value) {
    pointer[address] = (value >> 24) & 0xFF;
    pointer[address + 1] = (value >> 16) & 0xFF;
    pointer[address + 2] = (value >> 8) & 0xFF;
    pointer[address + 3] = value & 0xFF;
}

void writeHalfWord(uint8_t *pointer, uint32_t address, uint32_t value) {
    pointer[address] = (value >> 8) & 0xFF;
    pointer[address + 1] = value & 0xFF;
}

void writeByte(uint8_t *pointer, uint32_t address, uint32_t value) {
    pointer[address] = value & 0xFF;
}
uint32_t fetchInstructionWord(uint8_t *pointer, uint32_t address) {
    return (pointer[address] << 24) | 
    (pointer[address + 1] << 16) | 
    (pointer[address + 2] << 8) | 
    pointer[address + 3];
}

uint32_t fetchWord(uint8_t *pointer, uint32_t address) {
    return (pointer[address] << 24) | 
    (pointer[address + 1] << 16) | 
    (pointer[address + 2] << 8) | 
    pointer[address + 3];
}

uint32_t fetchHalfWord(uint8_t *pointer, uint32_t address) {
    return (pointer[address] << 8) | 
    (pointer[address + 1]);
}

uint32_t fetchByte(uint8_t *pointer, uint32_t address) {
    return pointer[address];
}
    
uint32_t signExtend(uint32_t value) {
    if((value & 0x0080) == 0x0080) {
        value = (value | 0xFFFF0000);
    }
    else {
        value = (value & 0x0000FFFF);
    }
    return value;
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
        decode.rX = (instruction & 0x01F00000) >> 20;
        decode.rY = (instruction & 0x00007C00) >> 10;
        decode.rZ = (instruction & 0x0000001F);
        decode.isImmediate = false;
    }
    else { //immediate operation
        decode.tRegisters[0] = cpuCore->registers[(instruction & 0x01F00000) >> 20];
        decode.tRegisters[1] = cpuCore->registers[(instruction & 0x00007C00) >> 10];
        decode.tRegisters[3] = (instruction & 0x0000FFFF);
        decode.rX = (instruction & 0x01F00000) >> 20;
        decode.rY = (instruction & 0x00007C00) >> 10;
        decode.isImmediate = true;
    }
    return decode;
}

pipelineInstruction instructionExecute(struct CPU *cpuCore, pipelineInstruction execute) {
    if (execute.isImmediate == false) {
        switch ((execute.instFetch >> 24)) { // operation type
            case 0x00: // LOADW

             break;
            case 0x02: //STOREW

             break;
            case 0x04: //LODH

             break;
            case 0x06: //STRH

             break;
            case 0x08: //LODB

             break;
            case 0x0A: //STRB

             break;
            case 0x0C: //MOV
                execute.tRegisters[0] = execute.tRegisters[1];
             break;
            case 0x0E: //SWAP
                uint32_t temp = execute.tRegisters[0];
                execute.tRegisters[0] = execute.tRegisters[1];
                execute.tRegisters[1] = temp;
             break;
            case 0x10: //LFR
                execute.tRegisters[0] = cpuCore->flagRegister;
             break;
            case 0x12: //SFR
                cpuCore->flagRegister = execute.tRegisters[0];
             break;
            case 0x14: //WCL
             break;
            case 0x16: //LLI
                execute.tRegisters[0] = (execute.tRegisters[3]);
             break;
            case 0x18: //LUI
                execute.tRegisters[0] = (execute.tRegisters[3] << 16);
             break;
            case 0x1A: //LSI
                execute.tRegisters[0] = signExtend(execute.tRegisters[3]);
             break;
            case 0x1C: //LDO
                execute.tRegisters[0] = fetchWord(cpuCore->dCache, execute.tRegisters[1] + signExtend(execute.tRegisters[3]));
             break;
            case 0x1E: //STO
                writeWord(cpuCore->dCache, execute.tRegisters[1] + signExtend(execute.tRegisters[3]), execute.tRegisters[0]);
             break;         
        }
    }
    else {
        switch(execute.instFetch >> 26) {
            case 0xAC: //LLI
                execute.tRegisters[0] = (execute.tRegisters[3]);
            break;
            case 0xB0: //LUI
                execute.tRegisters[0] = (execute.tRegisters[3] << 16);
            break;
            case 0xB4: //LSI
                execute.tRegisters[0] = signExtend(execute.tRegisters[3]);
            break;
        }
    }
    return execute;
}

pipelineInstruction instructionMemory(struct CPU *cpuCore, pipelineInstruction memory) {
    if (memory.isImmediate == false) {
        switch(memory.instFetch >> 25) {
            case 0x01: //LOADW
                memory.tRegisters[0] = fetchWord(cpuCore->dCache, memory.tRegisters[1]);
            break;
            case 0x02: //STOREW
                writeWord(cpuCore->dCache, memory.tRegisters[0], memory.tRegisters[1]);
            break;
            case 0x03: //LODH
                memory.tRegisters[0] = fetchHalfWord(cpuCore->dCache, memory.tRegisters[1]);
            break;
            case 0x04: //STRH
                writeHalfWord(cpuCore->dCache, memory.tRegisters[0], memory.tRegisters[1]);
            break;
            case 0x05: //LODB
                memory.tRegisters[0] = fetchByte(cpuCore->dCache, memory.tRegisters[1]);
            break;
            case 0x06: //STRB
                writeByte(cpuCore->dCache, memory.tRegisters[0], memory.tRegisters[1]);
            break;
        }
    }
    else {
        switch(memory.instFetch >> 26) {} // remember that the first bit is always 1

    }
    return memory;
}

pipelineInstruction instructionWriteBack(struct CPU *cpuCore, pipelineInstruction writeBack) {
    cpuCore->registers[writeBack.rX] = writeBack.tRegisters[0];
    return writeBack;
}

int execCycle(struct CPU *cpuCore) {

    cpuCore->pipelineStage[4] = instructionWriteBack(cpuCore, cpuCore->pipelineStage[3]);
    cpuCore->pipelineStage[3] = instructionMemory(cpuCore, cpuCore->pipelineStage[2]);
    cpuCore->pipelineStage[2] = instructionExecute(cpuCore, cpuCore->pipelineStage[1]);
    cpuCore->pipelineStage[1] = instructionDecode(cpuCore, cpuCore->pipelineStage[0]);
    cpuCore->pipelineStage[0] = instructionFetch(cpuCore);

    printf("new cycle-----------------------------\n");
    printf("PC: 0x%08X\n\n", cpuCore->pc);
    printPipelineStage(cpuCore->pipelineStage[0], "Fetch:");
    printPipelineStage(cpuCore->pipelineStage[1], "Decode:");
    printPipelineStage(cpuCore->pipelineStage[2], "Execute:");
    printPipelineStage(cpuCore->pipelineStage[3], "Memory:");
    printPipelineStage(cpuCore->pipelineStage[4], "Write Back:");
    printf("--------------------------------------\n\n");

    return 0;
}

void printPipelineStage(struct pipelineInstruction stage, const char *stageName) {
    printf("%s\n", stageName);
    printf("Instruction: 0x%08X\n", stage.instFetch);
    printf("Status: 0x%02X |  rX: %d |  rY: %d |  rZ: %d\n", stage.status, stage.rX, stage.rY, stage.rZ);
    printf("T0: 0x%08X  |  ", stage.tRegisters[0]);
    printf("T1: 0x%08X\n", stage.tRegisters[1]);
    printf("T2: 0x%08X  |  ", stage.tRegisters[2]);
    printf("T3: 0x%08X\n\n", stage.tRegisters[3]);
}