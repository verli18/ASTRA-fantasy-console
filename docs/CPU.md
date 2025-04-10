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

### Register operation

| Byte 1 | Byte 2 | Byte 3 | Byte 4 | 
| :-: | :-: | :-: | :-: 
| 0.nnnnnn.x | xxxx.mm.oo | yyyyy.mm.o | o.zzzzz.00 |

| Bits - byte 1 | description |
| :-: | :-: |
| 0 | always 0 for register operations |
| 1-6 | opcode |
| 7 | upper bit for rX index |

| Bits - byte 2 | description |
| :-: | :-: |
| 0-3 | lower 4 bits for rX index |
| 4-5 | offset mode for rX |
| 6-7 | offset value for rX ( from 1 to 4)|

| Bits - byte 3 | description |
| :-: | :-: |
| 0-4 | rY register index |
| 5-6 | offset mode for rY |
| 7 | upper bit for the offset value for rY |

| Bits - byte 4 | description |
| :-: | :-: |
| 0 | lower bit for the offset value for rY |
| 1-5 | rZ index |
| 6-7 | unused for now |

The offset mode allows for algorithms that "scan" an area of memory, and also a software implementation of a stack.

### Immediate operation

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

As many other parts of this system, I have not finalized this yet ( I know, probably like the 5th time I said this), so please don't think too deeply about this ( and if you do, please tell them to me!! I'm desperate for feedback on this lol).

The ammount of cycles needed to access RAM has not yet been decided, but it'll be significant for sure, because of that, the CPU has access to 4kb of instruction cache and 4kb of data cache. The cache uses 16 byte cache lines and utilizes direct mapping, it is also write deferred, as accessing memory would be too slow for write through and could clog up the main memory bus.


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


## Instructions

