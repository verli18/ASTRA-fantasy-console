#riscv32-unknown-elf-gcc -nostdlib -march=rv32i -mabi=ilp32 -T linker.lds -o RISCV RISCV.s
    .global _start

_start:
    li a2, 5
    li a3, 4
    add a1, a2, a3
    li a3, 0
    li a2, 0
    lui a3, 0x20408
    auipc t2, 0x00
    add a3, a3, t2
    li a1, 10
    lui a4, 0x00004
    addi a4, zero, 0xF0
    sb a4, 0(a1)
    addi a4, a4, 0x04
    sh a4, 0(a1)
    addi a4, a4, 0x04
    sw a4, 0(a1)

stop: 
    addi a3, a3, 1
    j stop
