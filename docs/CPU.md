# The CPU Core! ( still unamed)

The CPU is a 32-bit RISC based design, it is currently design to have the following features:

- 32 general purpose registers
- 32-bit address space, with 2mb of RAM ( although not particularly fast memory)
- 4kb of instruction cache and 4kb of data cache
- complete ALU, with fast division and MAC instructions
- immediate and register based instructions
- simple design, easy to learn

## Instruction types:

rX, rY and rZ are shorthand for register indexes, as a rule of thumb, register operations will kind of follow the pattern of rX being the destination register, rY being the first operand and rZ being the second one. And for immediate type operations, rZ is replaced by an immediate value, which will be the second operand.

### Register operation ( rOP types)

| Byte 1 | Byte 2 | Byte 3 | Byte 4 | 
| :-: | :-: | :-: | :-: 
| 0.nnnnnn.x | xxxx.mm.oo | o.yyyyy.mm | ooo.zzzzz |

| Bits - byte 1 | description |
| :-: | :-: |
| 0 | always 0 for register operations |
| 1-6 | opcode |
| 7 | upper bit for rX index |

| Bits - byte 2 | description |
| :-: | :-: |
| 0-3 | lower 4 bits for rX index |
| 4-5 | offset mode for rX |
| 6-7 | upper 2 bits for the offset value for rX ( from 1 to 8)|

| Bits - byte 3 | description |
| :-: | :-: |
| 0 | lower bit for the offset value for rX |
| 1-5 | rY register index |
| 6-7 | offset mode for rY |

| Bits - byte 4 | description |
| :-: | :-: |
| 0-2 | offset value for rY |
| 3-7 | rZ index |

The offset mode allows for algorithms that "scan" an area of memory, and also a software implementation of a stack. If the instruction only uses a single operand , the rZ bits are ignored. There are also some instructions that don't use operands, in that case, the rY bits are also unused.

Offset modes:

| Mode | Description |
| :-: | :-: |
| 00 | no offset |
| 01 | increment and modify register |
| 10 | decrement and modify register |
| 11 | increment (not modifying the register) |


### Immediate operation (iOP type)

| Byte 1 | Byte 2 | Byte 3 | Byte 4 | 
| :-: | :-: | :-: | :-: 
| 1.nnnnn.xx | xxx.yyyyy | immediate 16-bit signed/unsigned value | - |

| Bits - byte 1 | description |
| :-: | :-: |
| 0 | always 1 for immediate operations |
| 1-5| opcode |
| 6-7 | upper 2 bits for rX |

| Bits - byte 2 | description |
| :-: | :-: |
| 0-2 | lower 3 bits for rX |
| 3-7 | rY index |

Byte 3 and 4 are an immediate value, it may be signed or unsigned, depends on the instruction.


## Cache

Since ASTRA uses 24-bit addresses and the cache lines are 16 bytes wide
the tag is 12 bits wide and the index is 8 bits wide. The cache has a separate piece of memory for the tags:

| bits | description |
| :-: | :-: |
| 0-1 | unused |
| 2-3 | dirty/valid bit |
| 4-15 | tag |

## Registers

None of these registers are special, these names and uses are more of a standard, do expect bios routines and libraries written by me to use them like this though!

| Register number | Name | Register number | Name |
| :-: | :-: | :-: | :-: |
| 0x00 | zero | 0x10 | function argument 1 |
| 0x01 | Pivot | 0x11 | function argument 2 |
| 0x02 | free register | 0x12 | function argument 3 |
| 0x03 | free register | 0x13 | function argument 4 |
| 0x04 | free register | 0x14 | functio-safe |
| 0x05 | free register | 0x15 | function-safe |
| 0x06 | free register | 0x16 | function-safe |
| 0x07 | free register | 0x17 | function-safe |
| 0x08 | function return 1 | 0x18 | function-safe |
| 0x09 | function return 2 | 0x19 | function-safe |
| 0x0A | function return 3 | 0x1A | function-safe |
| 0x0B | function return 4 | 0x1B | function-safe |
| 0x0C | reserved for error/exception handling | 0x1C | free register |
| 0x0D | reserved for error/exception handling | 0x1D | return pointer |
| 0x0E | multiply/divide high register | 0x1E | interrupt pointer |
| 0x0F | multiply/divide low register | 0x1F | stack pointer |

There is also the flag register:
| Bit | Description |
| :-: | :-: |
| 0 | carry |
| 1 | borrow |
| 2 | less than |

## Instructions

Instuctions tend to follow the pattern of rX being the result register, rY being the first operand and rZ being the second operand.

**NOTE:** For the docs, I made a few abbreviations for the sake of not having to write things like "the pointer inside register X", here are some of them:

* rX, rY, rZ -> generally the result register, first operand and second operand in order. Some times this rule is broken for single operand or single register instructions, but it's a good rule of thumb for programming the ASTRA core. 
* rN -> describe the Nth register, is also used as just shorthand for any register.  
* imm16 -> a immediate 16-bit value, generally used for immediate operations.  
* sImm16 -> treats the immediate value as a signed number, generally used for offset-related instructions.  
* *rN -> treats the value of rN as a pointer.  

### Memory operations

| Instruction | type | description |
| :-: | :-: | :-: |
| LODW | rOP | loads a word from address *rY into rX |
| STRW | rOP | stores the word of rY into *rX |
| LODH | rOP | loads a half-word from address *rY into rX |
| STRH | rOP | stores the half-word of rY into *rX |
| LODB | rOP | loads a byte from address *rY into rX|
| STRB | rOP | stores the first byte of rY into *rX |
| MOV | rOP | moves the value of rY into rX |
| SWAP | rOP | swaps the values of rX and rY |
| LFR | rOP | Loads the contents of the flag register into rX |
| SFR | rOP | Stores the contents of rX into the flag register |
| WCL | rOP | if the cache line of *rX is dirty, write that back to memory now |
| LLI | iOP | loads a imm16 value into the lower 16 bits of rX |
| LUI | iOP | loads a imm16 value into the upper 16 bits of rX |
| LSI | iOP | loads a sign-extended imm16 value into rX |
| LDO | iOP | loads a value into rX from *rY + sImm16 |
| STO | iOP | stores the value of rX into *rY + sImm16 |

### Jump operations

| Instruction | type | description |
| :-: | :-: | :-: |
| JMP | iOP | jumps to *rX + sImm16 |
| JRT | rOP | jumps to *rX and saves the current address to rY |
| JPE | rOP | jumps to *rX if rY == rZ, sets/resets the equal bit on the flag register |
| JPL | rOP | jumps to *rX if rY < rZ , sets/resets the less than bit on the flag register |
| CMP | rOP | compares rY and rZ and sets the flag register accordingly |
| JPF | iOP | jumps to *rX if the Nth bit of the flag register is set ( N being an immediate value) |
| JPNF | iOP | jumps to *rX if the Nth bit of the flag register is not set |

### ALU operations

| Instruction | type | description |
| :-: | :-: | :-: |
| ADD | rOP | rX = rY + rZ |
| SUB | rOP | rX = rY - rZ |
| MUL | rOP | rX = rY * rZ |
| AND | rOP | rX = rY & rZ |
| OR | rOP | rX = rY \|\| rZ |
| XOR | rOP | rX = rY ^ rZ |
| NOT | rOP | rX = !rY |

##Control ROM

The control ROM is 256 words wide, it contains control signals for ASTRA, for all practical purposes, the programmer will never need to know or think about this, but I need to document it for programming the emulator.

| bits | description |
| :-: | :-: |
| 0 | always 1 |
| 1-2 | type of instruction (math, jump, memory) |
| 3-6 | if it uses the ALU, what operation? (none, add, sub, and, or, xor, not, lshift, rshift, lrot, rrot, sign-extend) |
| 7 | rOP/iOP select |






