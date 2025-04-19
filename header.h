#include "raylib.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"

#define cpuStartVec 0x00000000
#define DIRTY_BIT 0x2000
#define VALID_BIT 0x1000
#define CACHE_SIZE 4096
#define regX 0
#define regY 1
#define regZ 2
#define imm 3

typedef struct pipelineInstruction {
    uint32_t status;
    uint8_t stage;
    uint32_t instFetch;
    uint32_t tRegisters[4];
    uint8_t rX, rY, rZ;
    bool isImmediate;
} pipelineInstruction;

typedef struct CPU {
    uint32_t registers[32];
    pipelineInstruction pipelineStage[5];
    uint32_t pc;
    uint32_t flagRegister;
    uint8_t *iCache;
    uint8_t *iCacheTags;
    uint8_t *dCache;
    uint8_t *dCacheTags;
} cpu;

//array of instruction functions
typedef pipelineInstruction(*instFunc)(cpu*, pipelineInstruction);

//instruction functions
pipelineInstruction LODW(cpu *cpuCore, pipelineInstruction inst);

//helper functions
bool isdCacheHit(struct CPU *cpuCore, uint32_t address);
static inline uint32_t getTag(uint32_t address) {
    return (address >> 12) & 0x00000FFF;}
    
static inline uint32_t getLineIndex(uint32_t address) {
    return (address >> 4) & 0xFF;}

//memory related functions
uint32_t fetchInstructionWord(struct CPU *cpuCore, uint32_t address);
uint32_t fetchWord(struct CPU *cpuCore, uint32_t address);
uint32_t fetchHalfWord(struct CPU *cpuCore, uint32_t address);
uint32_t fetchByte(struct CPU *cpuCore, uint32_t address);
void writeWord(struct CPU *cpuCore, uint32_t address, uint32_t value);
void writeHalfWord(struct CPU *cpuCore, uint32_t address, uint32_t value);
void writeByte(struct CPU *cpuCore, uint32_t address, uint32_t value);

//math functions
uint32_t signExtend(uint32_t value);

//other functions
void initConsole(struct CPU *cpuCore, FILE *bin);
void initInstructionArray(instFunc *array);
//execution related functions
pipelineInstruction instructionFetch( struct CPU *cpuCore);
pipelineInstruction instructionDecode(struct CPU *cpuCore, pipelineInstruction decode);
pipelineInstruction instructionExecute(struct CPU *cpuCore, pipelineInstruction execute);
int execCycle(struct CPU *cpuCore);

//debug functions
void printPipelineStage(pipelineInstruction stage, const char *stageName);
