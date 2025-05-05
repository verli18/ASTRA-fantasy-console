#include "header.h"
#include <stdio.h>

void initConsole(cpu *cpuCore, FILE *bin) {
  InitWindow(640, 480, "ASTRA");
  // TODO: Implement cache logic
  cpuCore->iCache = calloc(CACHE_SIZE, sizeof(uint8_t));
  cpuCore->iCacheTags = calloc(256, 2);
  cpuCore->dCache = calloc(CACHE_SIZE, sizeof(uint8_t));
  cpuCore->dCacheTags = calloc(256, 2);
  cpuCore->pc = CPUSTARTVEC;
 for (int i = 0; i < 32; i++) {
    cpuCore->registers[i] = 0;
  }

  for(int i = 0; i < 5; i++){
    cpuCore->pipelineStage[i] = clearPipelineStage(cpuCore->pipelineStage[i]);
  }
  
  cpuCore->nop = clearPipelineStage(cpuCore->nop);
  cpuCore->nop.instFetch = NOP;
  fread(cpuCore->iCache, 1, CACHE_SIZE, bin);
}

pipelineInstruction clearPipelineStage(pipelineInstruction inst){
  for(int i = 0; i < 4; i++){ inst.tRegisters[i] = 0;}
  inst.rs1 = 0;
  inst.rs2 = 0;
  inst.rd = 0;
  inst.funct3 = 0;
  inst.funct7 = 0;
  inst.stall = 0;
  inst.pcFetch = 0;
  inst.mnemonic = NULL;
  inst.instFetch = 0;

  return inst;
}
bool isdCacheHit(cpu *cpuCore, uint32_t address) {
  return (((cpuCore->dCacheTags[getLineIndex(address)] << 8) |
           cpuCore->dCacheTags[getLineIndex(address) + 1]) &
          0x0FFF) == getTag(address);
}

uint32_t fetchInstructionWord(cpu *cpuCore, uint32_t address) {
  return (cpuCore->iCache[address + 3] << 24) |
         (cpuCore->iCache[address + 2] << 16) |
         (cpuCore->iCache[address + 1] << 8) | cpuCore->iCache[address];
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
  for(int i = 0; i < 4; i++){fetch.tRegisters[i] = 0;} //not sure if this is optimal, but it'll do

  cpuCore->pc += 4;
  fetch.status = 1;
  fetch.stall = 0;
  fetch.stage = 0;
  fetch.pcFetch = cpuCore->pc;

  return fetch;
}

pipelineInstruction instructionDecode(cpu *cpuCore, pipelineInstruction decode) {
    uint32_t inst = decode.instFetch;
    uint32_t rs1, rs2, rd, funct3, funct7, imm;
    rs2 = 0; // WARNING: pretty sure this breaks the zero-ness of r0, but will need to think abut it later
    rs1 = 0;
    rd = 0; 
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
                            ((inst & 0x80) << 7) | ((inst & 0x80000000) >> 16));
     break;
    case jal:  //why does risc-v make decoding this so hard???
    rd = (inst & 0xF80) >> 7;
    // Reconstructed as: imm[20|10:1|11|19:12]
    imm = ((inst >> 31) << 20) |     // imm[20]
          ((inst >> 12) & 0xFF) << 12 |  // imm[19:12]
          ((inst >> 20) & 0x1) << 11 |   // imm[11]
          ((inst >> 21) & 0x3FF) << 1;   // imm[10:1]
    
    // Sign extend
    if (imm & 0x100000){  // If bit 20 is set (sign bit)
        imm |= 0xFFF00000;
    }  // Extend sign
     break;
    case jalr:
        rs1 = (inst & 0xF8000) >> 15;
        rd = (inst & 0xF80) >> 7;
        imm = signExtend12((inst & 0xFFF00000) >> 20);
        funct3 = (inst & 0x7000) >> 12;
     break;
    case lui:
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
     case 0x00:
        printf("\nnull instruction in decode stage!!");
        return decode;
      break;
     default:
      printf("\ninvalid instruction in decode stage!!");
      return decode;
      break;
    }

    decode.rs1 = rs1;
    decode.rs2 = rs2;
    decode.rd = rd;
    decode.funct3 = funct3;
    decode.funct7 = funct7;
    
    decode.tRegisters[RS1] = cpuCore->registers[rs1];
    decode.tRegisters[RS2] = cpuCore->registers[rs2];
    decode.tRegisters[IMM] = imm;

    decode.stage = 1;
    if(decode.stall == 0){
    if(((cpuCore->pipelineStage[2].rd == decode.rs1) || (cpuCore->pipelineStage[2].rd == decode.rs2)) && (cpuCore->pipelineStage[2].rd != 0)){
      decode.stall = 3;
    }
    else if(((cpuCore->pipelineStage[3].rd == decode.rs1) || (cpuCore->pipelineStage[3].rd == decode.rs2)) && (cpuCore->pipelineStage[3].rd != 0)){
      decode.stall = 2;
    }
    else if(((cpuCore->pipelineStage[4].rd == decode.rs1) || (cpuCore->pipelineStage[4].rd == decode.rs2)) && (cpuCore->pipelineStage[4].rd != 0)) {
      decode.stall = 1;
    }
    }
    if (cpuCore->pipelineStage[1].stall != 0) {
      cpuCore->pipelineStage[1].stall--;
    }
  return decode;
}

pipelineInstruction instructionExecute(cpu *cpuCore, pipelineInstruction execute) {
  uint32_t rd;
  uint32_t rs1 = execute.tRegisters[RS1];
  uint32_t rs2 = execute.tRegisters[RS2];
  uint32_t imm = execute.tRegisters[IMM];

  switch (execute.opCode) {
    case mReg:
      if(execute.funct7 == 0b00100000) {
        if(execute.funct3 == 0b000){
          rd = rs1 - rs2;
          execute.mnemonic = "sub";
        }
        else {
          rd = (int32_t)rs1 >> rs2;
          execute.mnemonic = "rsa";
        }
      }
      else {
      switch(execute.funct3){
        case 0b000:
          rd = rs1 + rs2;
          execute.mnemonic = "add";
          break;
        case 0b100:
          rd = rs1 ^ rs2;
          execute.mnemonic = "xor";
         break;
        case 0b110:
          rd = rs1 | rs2;
          execute.mnemonic = "or";
         break;
        case 0b111:
          rd = rs1 & rs2;
          execute.mnemonic = "and";
         break;
        case 0b001:
          rd = rs1 << rs2;
          execute.mnemonic = "sll";
         break;
        case 0b101:
          rd = rs1 >> rs2;
          execute.mnemonic = "srl";
         break;
        case 0b010:
          (rs1 < rs2)? (rd = 1) : (rd = 0);
          execute.mnemonic = "stl";
        break;
        }
      }

      break;
    case mImm:
      switch (execute.funct3) {
        case 0b000: 
          rd = rs1 + imm;
          execute.mnemonic = "addi";
         break;
        case 0b100: //apparently RISC-V just does immediated bitwise logic with sign-extended values? wacky..
          rd = rs1 ^ imm;
          execute.mnemonic = "xori";
         break;
        case 0b110:
          rd = rs1 | imm;
          execute.mnemonic = "ori";
         break;
        case 0b111:
          rd = rs1 & imm;
          execute.mnemonic = "andi";
          break;
        case 0b001:
          rd = rs1 << imm;
          execute.mnemonic = "slli";
         break;
        case 0b101:
          if((imm & 0xFE0) != 0){
            rd = (int)rs1 >> imm;
            execute.mnemonic = "srai";
          }
          else {
            rd = rs1 >> imm;
            execute.mnemonic = "srli";
          }
         break;
      }
     break;
    case load:
      switch(execute.funct3){
        case 0b000:
          execute.mnemonic = "lb";
         break;
        case 0b001:
          execute.mnemonic = "lh";
         break;
        case 0b010:
          execute.mnemonic = "lw";
         break;
        case 0b100:
          execute.mnemonic = "lbu";
         break;
        case 0b101:
          execute.mnemonic = "lhu";
         break;
      }
     break;
    case store:
      switch(execute.funct3){
        case 0b000:
          execute.mnemonic = "sb";
         break;
        case 0b001: 
          execute.mnemonic = "sh";
         break;
        case 0b010:
          execute.mnemonic = "sw";
         break;
      }
     break;
    case branch:
      switch(execute.funct3){
        case 0b000:
          if(rs1 == rs2) {
            cpuCore->pc += (imm << 1);

            cpuCore->pipelineStage[0] = cpuCore->nop;
            cpuCore->pipelineStage[1] = cpuCore->nop;
          }

          execute.mnemonic = "beq";
         break;
        case 0b001:
          if(rs1 != rs2){
            cpuCore->pc += (imm << 1);

            cpuCore->pipelineStage[0] = cpuCore->nop;
            cpuCore->pipelineStage[1] = cpuCore->nop;
          }
          execute.mnemonic = "bne";
         break;
        case 0b100:
          if(rs1 < rs2){
            cpuCore->pc += (imm << 1);

            cpuCore->pipelineStage[0] = cpuCore->nop;
            cpuCore->pipelineStage[1] = cpuCore->nop;
          }
          execute.mnemonic = "blt";
         break;
        case 0b101:
          if(rs1 >= rs2){
            cpuCore->pc += (imm << 1);

            cpuCore->pipelineStage[0] = cpuCore->nop;
            cpuCore->pipelineStage[1] = cpuCore->nop;
          }
          execute.mnemonic = "bge";
         break;
      }
     break;
    case jal:
      rd = execute.pcFetch + 4;
      cpuCore->pc = execute.pcFetch + imm - 4;
      execute.mnemonic = "jal";

      cpuCore->pipelineStage[0] = cpuCore->nop;
      cpuCore->pipelineStage[1] = cpuCore->nop;
     break;
    case jalr:
      rd = execute.pcFetch + 4;

      cpuCore->pc = rs1 + imm;
      execute.mnemonic = "jalr";

      cpuCore->pipelineStage[0] = cpuCore->nop;
      cpuCore->pipelineStage[1] = cpuCore->nop;
     break;
    case lui:
      rd = imm << 12;

      execute.mnemonic = "lui";
     break;
    case auipc:
      rd = execute.pcFetch + imm;
      execute.mnemonic = "auipc";
     break;
    case sys:
     break;
    }
  if(execute.instFetch == NOP){execute.mnemonic = "nop";}
  execute.tRegisters[RD] = rd; 
  execute.stage = 2;
  return execute;
}

pipelineInstruction instructionMemory(cpu *cpuCore, pipelineInstruction memory){

  return memory;
}

pipelineInstruction instructionWrite(cpu *cpuCore, pipelineInstruction write){
  switch(write.opCode){
    default:
      (write.rd == 0)? ({cpuCore->registers[0] = 0;}) : ({cpuCore->registers[write.rd] = write.tRegisters[RD];});
     break;
    case store:
     break;
    case branch:
     break;
  }
  return write;
}

int execCycle(cpu *cpuCore) {
  
  cpuCore->pipelineStage[4] = instructionWrite(cpuCore, cpuCore->pipelineStage[3]);
  cpuCore->pipelineStage[3] = instructionMemory(cpuCore, cpuCore->pipelineStage[2]);
  if(cpuCore->pipelineStage[1].stall == 0){
    cpuCore->pipelineStage[2] = instructionExecute(cpuCore,cpuCore->pipelineStage[1]);
  }
  else {
    cpuCore->pipelineStage[2] = instructionExecute(cpuCore, cpuCore->nop);
  }
  cpuCore->pipelineStage[1] = instructionDecode(cpuCore,cpuCore->pipelineStage[0]);
   if(cpuCore->pipelineStage[1].stall == 0){
    cpuCore->pipelineStage[0] = instructionFetch(cpuCore);
  }

  return 0;
}

