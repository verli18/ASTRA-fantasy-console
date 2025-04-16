#define cpuStartVec 0x00000000

struct pipelineStage {
    uint32_t status;
    uint32_t instFetch;
    uint32_t tRegisters[4];
};

struct CPU {
    uint32_t registers[32];
    struct pipelineStage pipelineStage[5];
    uint32_t pc;
    uint32_t *iCache;
    uint32_t *iCacheTags;
    uint8_t *dCache;
    uint8_t *dCacheTags;
};

void initConsole(struct CPU *cpuCore, FILE *bin){
    InitWindow(640, 480, "ASTRA");
    //TODO: Implement cache logic
    cpuCore->iCache = calloc(1024, sizeof(uint32_t));
    cpuCore->dCache = calloc(4096, 1);
    cpuCore->pc = cpuStartVec;
    for(int i = 0; i < 32; i++) {
        cpuCore->registers[i] = i;
    }
    
    fread(cpuCore->iCache, 4, 1024, bin);
}

struct pipelineStage instructionFetch( struct CPU *cpuCore) {
    struct pipelineStage fetch;
    fetch.instFetch = cpuCore->iCache[cpuCore->pc];
    cpuCore->pc += 1;
    fetch.status = 1;
    return fetch;
}

struct pipelineStage instructionDecode(struct CPU *cpuCore, struct pipelineStage decode) {
    int instruction = decode.instFetch;

    if ((instruction & 0x80000000) == 0) { //register operation
        decode.tRegisters[0] = cpuCore->registers[(instruction & 0x01F00000) >> 20];
        decode.tRegisters[1] = cpuCore->registers[(instruction & 0x00007C00) >> 10];
        decode.tRegisters[2] = cpuCore->registers[(instruction & 0x0000001F)];
    }
    return decode;
}


void printPipelineStage(struct pipelineStage stage, const char *stageName) {
    printf("%s\n", stageName);
    printf("Instruction: 0x%08X\n", stage.instFetch);
    printf("Status: 0x%02X\n", stage.status);
    printf("T0: 0x%08X  |  ", stage.tRegisters[0]);
    printf("T1: 0x%08X\n", stage.tRegisters[1]);
    printf("T2: 0x%08X  |  ", stage.tRegisters[2]);
    printf("T3: 0x%08X\n\n", stage.tRegisters[3]);
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