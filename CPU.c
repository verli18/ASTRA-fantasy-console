#define cpuStartVec 0x00000000

struct pipelineStage {
    uint32_t status;
    uint32_t instFetch;
    uint32_t tRegisters[4];
};

struct CPU {
    uint32_t registers[32];
    uint32_t pc;
    int *iCache;
    int *iCacheTags;
    int *dCache;
    int *dCacheTags;
};

void initConsole(struct CPU *cpuCore, FILE *bin){
    InitWindow(640, 480, "ASTRA");
    //TODO: Implement cache logic
    cpuCore->iCache = (uint32_t*)malloc(4096 * sizeof(uint32_t));
    cpuCore->dCache = malloc(4096 * sizeof(uint32_t));
    cpuCore->pc = cpuStartVec;

    fread(cpuCore->iCache, sizeof(uint32_t), 4096, bin);
    
}

int instructionFetch( struct CPU *cpuCore, struct pipelineStage *InstFetch) {
    InstFetch->instFetch = cpuCore->iCache[cpuCore->pc];
    return 0;
}

int execCycle(struct CPU *cpuCore) {
    struct pipelineStage instFetch;
    instructionFetch(cpuCore, &instFetch);

    cpuCore->pc += 4;
    return 0;
}