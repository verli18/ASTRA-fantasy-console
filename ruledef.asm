
#bankdef ASTRA
{
    #addr 0x0000
    #size 0x1000
    #outp 0x0000
    #bits 32
}
#ruledef
{
    load r{x}, r{y} => 0`1 @ 0b000000`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    str r{x}, r{y} => 0`1 @ 0b000001`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    mov r{x}, r{y} => 0`1 @ 0b000010`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    swap r{x}, r{y} => 0`1 @ 0b000011`6 @ x`5 @ 0`5 @ y`5 @ 0`5 @ 0b0`5
    lfr r{x} => 0`1 @ 0b000100`6 @ x`5 @ 0`20
    sfr r{x} => 0`1 @ 0b000101`6 @ x`5 @ 0`20
    wcl r{x} => 0`1 @ 0b000110`6 @ x`5 @ 0`20
}

#bank ASTRA

load r1, r2
str r3, r4
mov r4, r5
swap r1, r2
lfr r3
sfr r4
wcl r5