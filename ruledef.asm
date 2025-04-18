
#bankdef ASTRA
{
    #addr 0x0000
    #size 0x1000
    #outp 0x0000
    #bits 32
}
#ruledef
{
    lodw r{x}, r{y} => 0`1 @ 0b000000`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    strw r{x}, r{y} => 0`1 @ 0b000001`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    lodh r{x}, r{y} => 0`1 @ 0b000010`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    strh r{x}, r{y} => 0`1 @ 0b000011`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    lodb r{x}, r{y} => 0`1 @ 0b000100`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    strb r{x}, r{y} => 0`1 @ 0b000101`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    mov r{x}, r{y} => 0`1 @ 0b000110`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    swap r{x}, r{y} => 0`1 @ 0b000111`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    lfr r{x} => 0`1 @ 0b001000`6 @ x`5 @ 0`20
    sfr r{x} => 0`1 @ 0b001001`6 @ x`5 @ 0`20
    wcl r{x} => 0`1 @ 0b001010`6 @ x`5 @ 0`20
    lli r{x}, {imm} => 1`1 @ 0b01011`5 @ x`5 @ 0`5 @ imm`16
    lui r{x}, {imm} => 1`1 @ 0b01100`5 @ x`5 @ 0`5 @ imm`16
    lsi r{x}, {imm} => 1`1 @ 0b01101`5 @ x`5 @ 0`5 @ imm`16
}

#bank ASTRA

lli r1, 0x1234
lui r1, 0xFFFF

strw r2, r1
lodw r3, r2
