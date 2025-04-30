#include "header.h"
// if some lines are weirdly chopped, it's because I can't figure out how to
// stop lazyvim from putting random fucking newlines everywhere
void initConsole(cpu *cpuCore, FILE *bin) {
  InitWindow(640, 480, "ASTRA");
  // TODO: Implement cache logic
  cpuCore->iCache = calloc(CACHE_SIZE, sizeof(uint8_t));
  cpuCore->iCacheTags = calloc(256, 2);
  cpuCore->dCache = calloc(CACHE_SIZE, sizeof(uint8_t));
  cpuCore->dCacheTags = calloc(256, 2);
  cpuCore->pc = cpuStartVec;
  for (int i = 0; i < 32; i++) {
    cpuCore->registers[i] = 0;
  }

  fread(cpuCore->iCache, 1, CACHE_SIZE, bin);
}

bool isdCacheHit(cpu *cpuCore, uint32_t address) {
  return (((cpuCore->dCacheTags[getLineIndex(address)] << 8) |
           cpuCore->dCacheTags[getLineIndex(address) + 1]) &
          0x0FFF) == getTag(address);
}

// these will need to be expanded to work on cache later
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

  if (isdCacheHit(cpuCore, address) || !dirty) {
    cpuCore->dCache[getLineIndex(address) + (address & 0xF)] = value;
    cpuCore->dCacheTags[getLineIndex(address)] = DIRTY_BIT | getTag(address);
  }
  if (dirty) {
    printf("Line is dirty, need to write to memory\n");
  }
}

uint32_t fetchInstructionWord(cpu *cpuCore, uint32_t address) {
  return (cpuCore->iCache[address] << 24) |
         (cpuCore->iCache[address + 1] << 16) |
         (cpuCore->iCache[address + 2] << 8) | cpuCore->iCache[address + 3];
}

uint32_t fetchWord(cpu *cpuCore, uint32_t address) {
  if (isdCacheHit(cpuCore, address)) {
    return (cpuCore->dCache[address] << 24) |
           (cpuCore->dCache[address + 1] << 16) |
           (cpuCore->dCache[address + 2] << 8) | cpuCore->dCache[address + 3];
  } else {
    printf("Cache miss, need to fetch from memory\n");
    return fetchInstructionWord(cpuCore, address);
  }
}

uint32_t fetchHalfWord(cpu *cpuCore, uint32_t address) {
  return (cpuCore->dCache[address] << 8) | (cpuCore->dCache[address + 1]);
}

uint32_t fetchByte(cpu *cpuCore, uint32_t address) {
  return cpuCore->dCache[address];
}

uint32_t signExtend20(uint32_t value) {
  if ((value & 0xFFFFF) != 0) {
    value = value | 0xFFF00000;
    } else {
        value = value & 0x000FFFFF;
    }
  return value;
}

uint32_t signExtend12(uint32_t value) {
  if ((value & 0x800) != 0) {
    value = value | 0xFFFFF000;
  } else {
    value = value & 0x00000FFF;
  }
  return value;
}

pipelineInstruction instructionFetch(cpu *cpuCore) {
  pipelineInstruction fetch;
  fetch.instFetch = fetchInstructionWord(cpuCore, cpuCore->pc);
  cpuCore->pc += 4;
  fetch.status = 1;
  return fetch;
}

pipelineInstruction instructionDecode(pipelineInstruction decode) {
    uint32_t inst = decode.instFetch;
    uint32_t rs1, rs2, rd, funct3, funct7, imm;

    decode.opCode = inst & 0b01111111;
    switch (decode.opCode) {
    case mReg:
        rs1 = (inst & 0xF8000) >> 15;
        rs2 = (inst & 0x1F00000) >> 20;
        rd = (inst & 0xF80) >> 7;
        funct7 = (inst & 0xFE000000) >> 25;
        funct3 = (inst & 0x7000) >> 12;
     break;
    case mImm:
        rs1 = (inst & 0xF8000) >> 15;
        rd = (inst & 0xF80) >> 7;
        imm = signExtend12((inst & 0xFFF00000) >> 20);
        funct3 = (inst & 0x7000) >> 12;
     break;
    case load:
        rs1 = (inst & 0xF8000) >> 15;
        rd = (inst & 0xF80) >> 7;
        imm = signExtend12((inst & 0xFFF00000) >> 20);
        funct3 = (inst & 0x7000) >> 12;
     break;
    case store:
        rs1 = (inst & 0xF8000) >> 15;
        rs2 = (inst & 0x1F00000) >> 20;
        funct3 = (inst & 0x7000) >> 12;
        imm = signExtend12( (decode.instFetch >> 20) | ( (decode.instFetch >> 7) & 0x1F));
     break;
    case branch:
        rs1 = (inst & 0xF8000) >> 15;
        rs2 = (inst & 0x1F00000) >> 20;
        funct3 = (inst & 0x7000) >> 12;
        imm = signExtend12( ((inst & 0xF00) >> 8) | ((inst & 0x7E000000) >> 17) |
                            ((inst & 0x80) << 7) | ((inst & 0x80000000) >> 16)) << 1;
     break;
    case jal:  //why does risc-v make decoding this so hard???
        rd = (inst & 0xF80) >> 7;
        imm = signExtend20( ((inst & 0x7FE00000) >> 21) | ((inst & 0x100000) >> 10) | 
                            (inst & 0xFF000) | ((inst & 0x80000000) >> 11)) << 1;
     break;
    case jalr:
        rs1 = (inst & 0xF8000) >> 15;
        rd = (inst & 0xF80) >> 7;
        imm = signExtend12((inst & 0xFFF00000) >> 20);
        funct3 = (inst & 0x7000) >> 12;
     break;
    case lui: // TODO: this needs to be sign extended 
        imm =  signExtend20(decode.instFetch >> 12);
        rd = (inst & 0xF80) >> 7;
     break;
    case auipc:
        imm =  signExtend20(decode.instFetch >> 12);
        rd = (inst & 0xF80) >> 7;
     break;
    case sys:
        rs1 = (inst & 0xF8000) >> 15;
        rd = (inst & 0xF80) >> 7;
        imm = signExtend12((inst & 0xFFF00000) >> 20);
        funct3 = (inst & 0x7000) >> 12;
     break;
    }

    decode.rs1 = rs1;
    decode.rs2 = rs2;
    decode.rd = rd;
    decode.funct3 = funct3;
    decode.funct7 = funct7;
    decode.tRegisters[IMM] = imm;

  return decode;
}

int execCycle(cpu *cpuCore) {

  cpuCore->pipelineStage[1] = instructionDecode(cpuCore->pipelineStage[0]);
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

void printPipelineStage(struct pipelineInstruction stage,
                        const char *stageName) {
  printf("%s\n", stageName);
  printf("Instruction: 0x%08X\n", stage.instFetch);
  printf("Status: 0x%02X |  rX: %d |  rY: %d |  rZ: %d\n", stage.status,
         stage.rs1, stage.rs2, stage.rd);
  printf("T0: 0x%08X  |  ", stage.tRegisters[0]);
  printf("T1: 0x%08X\n", stage.tRegisters[1]);
  printf("T2: 0x%08X  |  ", stage.tRegisters[2]);
  printf("T3: 0x%08X\n\n", stage.tRegisters[3]);
}
