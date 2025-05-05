#include "raylib.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

#define DEBUG_BLUE (Color){0, 0, 20, 240}
#define DEBUG_TEXT (Color){9, 127, 217, 255}
#define STEP_MODE true

#define CPUSTARTVEC 0x00001000
#define DIRTY_BIT 0x2000
#define VALID_BIT 0x1000
#define CACHE_SIZE 0xC000
#define RS1 0
#define RS2 1
#define RD 2
#define IMM 3

// opCodes
#define mImm 0b0010011
#define mReg 0b0110011
#define load 0b0000011
#define store 0b0100011
#define branch 0b1100011
#define jal 0b1101111
#define jalr 0b1100111
#define lui 0b0110111
#define auipc 0b0010111
#define sys 0b11100111
#define NOP 0x00000013

typedef struct pipelineInstruction {
  uint32_t status;
  uint8_t stage;
  uint32_t instFetch;
  uint32_t pcFetch;
  uint32_t tRegisters[4];
  uint8_t rs1, rs2, rd;
  uint8_t funct3, funct7;
  uint8_t opCode;
  uint8_t stall;
  char *mnemonic; //Will remove later, but makes it easier to read and debug.
} pipelineInstruction;

typedef struct cpu {
  uint32_t registers[32];
  pipelineInstruction pipelineStage[5];
  pipelineInstruction nop;
  uint32_t pc;
  uint32_t flagRegister;
  uint8_t *iCache;
  uint8_t *iCacheTags;
  uint8_t *dCache;
  uint8_t *dCacheTags;
} cpu;

// helper functions
bool isdCacheHit(struct cpu *cpuCore, uint32_t address);
static inline uint32_t getTag(uint32_t address) {
  return (address >> 12) & 0x00000FFF;
}

static inline uint32_t getLineIndex(uint32_t address) {
  return (address >> 4) & 0xFF;
}

// memory related functions
uint32_t fetchInstructionWord(struct cpu *cpuCore, uint32_t address);
uint32_t fetchWord(struct cpu *cpuCore, uint32_t address);
uint32_t fetchHalfWord(struct cpu *cpuCore, uint32_t address);
uint32_t fetchByte(struct cpu *cpuCore, uint32_t address);
void writeWord(struct cpu *cpuCore, uint32_t address, uint32_t value);
void writeHalfWord(struct cpu *cpuCore, uint32_t address, uint32_t value);
void writeByte(struct cpu *cpuCore, uint32_t address, uint32_t value);

// math functions
uint32_t signExtend(uint32_t value, uint8_t size);

// other functions
void initConsole(struct cpu *cpuCore, FILE *bin);
pipelineInstruction clearPipelineStage(pipelineInstruction inst);
// execution related functions

pipelineInstruction instructionFetch(cpu *cpuCore);
pipelineInstruction instructionDecode(cpu *cpuCore, pipelineInstruction decode);
pipelineInstruction instructionExecute(cpu *cpuCore, pipelineInstruction execute);
pipelineInstruction instructionMemory(cpu *cpuCore, pipelineInstruction memory);
pipelineInstruction instructionWrite(cpu *cpuCore, pipelineInstruction write);

int execCycle(struct cpu *cpuCore);

// debug functions
void drawRegisters(cpu *cpuCore);
void drawPipelineStages(cpu *cpuCore);
void drawStage(int yOffset, pipelineInstruction instruction, const char* instName);
