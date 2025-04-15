# things to be implemented

RV32! emulator:

## 32 registers

x0 hardwired all bits equal 0
x1-x31 general purpose registers
each register 32 bits wide

Table 2 shows the unprivileged state for the base integer ISA. For RV32I, the 32 x registers are each 32 bits wide, i.e., XLEN=32. Register x0 is hardwired with all bits equal to 0. General purpose registers x1x31 hold values that various instructions interpret as a collection of Boolean values, or as two’s complement signed binary integers or unsigned binary integers.


> Could have used the RV32E subset, with only 16 registers, but adding more registers is easy in terms of implementing the emulator

The base ISA has IALIGN=32,

RISC-V ISA keeps the source (rs1 and rs2) and destination (rd) registers at the same position in all format

immediates are always sign-extended, and are generally packed towards the leftmost available bits in the instruction
