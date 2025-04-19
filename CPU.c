#include "header.h"

void initConsole(cpu *cpuCore, FILE *bin){
    InitWindow(640, 480, "ASTRA");
    //TODO: Implement cache logic
    cpuCore->iCache = calloc(CACHE_SIZE, sizeof(uint8_t));
    cpuCore->iCacheTags = calloc(256, 2); 
    cpuCore->dCache = calloc(CACHE_SIZE, sizeof(uint8_t));
    cpuCore->dCacheTags = calloc(256, 2);
    cpuCore->pc = cpuStartVec;
    for(int i = 0; i < 32; i++) {
        cpuCore->registers[i] = 0;
    }

    fread(cpuCore->iCache, 1, CACHE_SIZE, bin);
}

void initInstructionArray(instFunc *array) {
    array[0] = LODW;
}

bool isdCacheHit(cpu *cpuCore, uint32_t address) {
    return (((cpuCore->dCacheTags[getLineIndex(address)] << 8) | cpuCore->dCacheTags[getLineIndex(address) + 1]) & 0x0FFF) == getTag(address);
}

//these will need to be expanded to work on cache later
void writeWord(cpu *cpuCore, uint32_t address, uint32_t value) {
    writeByte(cpuCore, address, value & 0xFF);
    writeByte(cpuCore, address + 1, (value >> 8) & 0xFF);
    writeByte(cpuCore, address + 2, (value >> 16) & 0xFF);
    writeByte(cpuCore, address + 3, (value >> 24) & 0xFF);
}

void writeHalfWord(cpu *cpuCore, uint32_t address, uint32_t value) {
    writeByte(cpuCore, address, value & 0xFF);
    writeByte(cpuCore, address + 1, (value >> 8) & 0xFF);
}

void writeByte(cpu *cpuCore, uint32_t address, uint32_t value) {
    bool dirty = (cpuCore->dCacheTags[getLineIndex(address)] & DIRTY_BIT);

    if(isdCacheHit(cpuCore, address) || !dirty) {
        cpuCore->dCache[getLineIndex(address) + (address & 0xF)] = value;
        cpuCore->dCacheTags[getLineIndex(address)] = DIRTY_BIT | getTag(address);
    }
    if(dirty) {
        printf("Line is dirty, need to write to memory\n");
    }
}
uint32_t fetchInstructionWord(cpu *cpuCore, uint32_t address) {
    return (cpuCore->iCache[address] << 24) | 
    (cpuCore->iCache[address + 1] << 16) | 
    (cpuCore->iCache[address + 2] << 8) | 
    cpuCore->iCache[address + 3];
}

uint32_t fetchWord(cpu *cpuCore, uint32_t address) {
    if(isdCacheHit(cpuCore, address)) {
        return (cpuCore->dCache[address] << 24) | 
        (cpuCore->dCache[address + 1] << 16) | 
        (cpuCore->dCache[address + 2] << 8) | 
        cpuCore->dCache[address + 3];
    }
    else {
        printf("Cache miss, need to fetch from memory\n");
        return fetchInstructionWord(cpuCore, address);
    }
}

uint32_t fetchHalfWord(cpu *cpuCore, uint32_t address) {
    return (cpuCore->dCache[address] << 8) | 
    (cpuCore->dCache[address + 1]);
}

uint32_t fetchByte(cpu *cpuCore, uint32_t address) {
    return cpuCore->dCache[address];
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
pipelineInstruction LODW(cpu *cpuCore, pipelineInstruction inst) {
    switch(inst.stage){
        case 1:
            inst = instructionFetch(cpuCore);
         break;
        case 2:
            inst = instructionDecode(cpuCore, inst);
         break;
        case 3:
         break;
        case 4:
         break;
        case 5:
         break;
    }
    inst.stage++;
    return inst;
}
pipelineInstruction instructionFetch( cpu *cpuCore) {
    pipelineInstruction fetch;
    fetch.instFetch = fetchInstructionWord(cpuCore, cpuCore->pc);
    cpuCore->pc += 4;
    fetch.status = 1;
    return fetch;
}

pipelineInstruction instructionDecode(cpu *cpuCore, pipelineInstruction decode) {
    int instruction = decode.instFetch;

    if ((instruction & 0x80000000) == 0) { //register operation
        decode.tRegisters[regX] = cpuCore->registers[(instruction & 0x01F00000) >> 20];
        decode.tRegisters[regY] = cpuCore->registers[(instruction & 0x00007C00) >> 10];
        decode.tRegisters[regZ] = cpuCore->registers[(instruction & 0x0000001F)];
        decode.rX = (instruction & 0x01F00000) >> 20;
        decode.rY = (instruction & 0x00007C00) >> 10;
        decode.rZ = (instruction & 0x0000001F);
        decode.isImmediate = false;
    }
    else { //immediate operation
        decode.tRegisters[regX] = cpuCore->registers[(instruction & 0x01F00000) >> 20];
        decode.tRegisters[regY] = cpuCore->registers[(instruction & 0x00007C00) >> 10];
        decode.tRegisters[imm] = (instruction & 0x0000FFFF);
        decode.rX = (instruction & 0x01F00000) >> 20;
        decode.rY = (instruction & 0x00007C00) >> 10;
        decode.isImmediate = true;
    }
    return decode;
}

pipelineInstruction instructionExecute(cpu *cpuCore, pipelineInstruction execute) {
    if (execute.isImmediate == false) {
        switch ((execute.instFetch >> 25)) { // operation type
            case 0x00: // LOADW

             break;
            case 0x01: //STOREW

             break;
            case 0x02: //LODH

             break;
            case 0x03: //STRH

             break;
            case 0x04: //LODB

             break;
            case 0x05: //STRB

             break;
            case 0x06: //MOV
                execute.tRegisters[regX] = execute.tRegisters[regY];
             break;
            case 0x07: //SWAP
                uint32_t temp = execute.tRegisters[regX];
                execute.tRegisters[regX] = execute.tRegisters[regY];
                execute.tRegisters[regY] = temp;
             break;
            case 0x08: //LFR
                execute.tRegisters[regX] = cpuCore->flagRegister;
             break;
            case 0x09: //SFR
                cpuCore->flagRegister = execute.tRegisters[regX];
             break;
            case 0x0A: //WCL
             break;
            case 0x0B: //LLI
                execute.tRegisters[regX] = (execute.tRegisters[imm]);
             break;
            case 0x0C: //LUI
                execute.tRegisters[regX] = (execute.tRegisters[imm] << 16);
             break;
            case 0x0D: //LSI
                execute.tRegisters[regX] = signExtend(execute.tRegisters[imm]);
             break;
            case 0x0E: //LDO
                execute.tRegisters[regX] = execute.tRegisters[regY] + signExtend(execute.tRegisters[imm]);
             break;
            case 0x0F: //STO
                execute.tRegisters[regX] = execute.tRegisters[regY] + signExtend(execute.tRegisters[imm]);
             break;         
        }
    }
    else {
        switch(execute.instFetch >> 26) {
            case 0x2B: //LLI
                execute.tRegisters[regX] = (execute.tRegisters[imm]);
            break;
            case 0x2C: //LUI
                execute.tRegisters[regX] = (execute.tRegisters[imm] << 16);
            break;
            case 0x2D: //LSI
                execute.tRegisters[regX] = signExtend(execute.tRegisters[imm]);
            break;
        }
    }
    return execute;
}

pipelineInstruction instructionMemory(cpu *cpuCore, pipelineInstruction memory) {
    if (memory.isImmediate == false) {
        switch(memory.instFetch >> 25) {
            case 0x01: //LOADW
                memory.tRegisters[regX] = fetchWord(cpuCore, memory.tRegisters[regY]);
            break;
            case 0x02: //STOREW
                writeWord(cpuCore, memory.tRegisters[regX], memory.tRegisters[regY]);
            break;
            case 0x03: //LODH
                memory.tRegisters[regX] = fetchHalfWord(cpuCore, memory.tRegisters[regY]);
            break;
            case 0x04: //STRH
                writeHalfWord(cpuCore, memory.tRegisters[regX], memory.tRegisters[regY]);
            break;
            case 0x05: //LODB
                memory.tRegisters[regX] = fetchByte(cpuCore, memory.tRegisters[regY]);
            break;
            case 0x06: //STRB
                writeByte(cpuCore, memory.tRegisters[regX], memory.tRegisters[regY]);
            break;
        }
    }
    else {
        switch(memory.instFetch >> 26) {} // remember that the first bit is always 1

    }
    return memory;
}

pipelineInstruction instructionWriteBack(cpu *cpuCore, pipelineInstruction writeBack) {
    cpuCore->registers[writeBack.rX] = writeBack.tRegisters[regX];
    return writeBack;
}

int execCycle(cpu *cpuCore) {

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