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



# Instruction tables:


### U-Type


| Immediate Offset`[31:12]` | rd (11:7) | opcode (6:0) | Instruction |
| ------------------------- | --------- | ------------ | ----------- |
| `imm[31:12]`              | rd        | 0110111      | LUI         |
| `imm[31:12]`              | rd        | 0010111      | AUIPC       |

### I-Type


| Immediate Offset`[31:20]` | Source Register rs1`[19:15]` | funct3`[14:12]` | Destination Register rd`[11:7]` | Opcode`[6:0]` | Instruction |
| ------------------------- | ---------------------------- | --------------- | ------------------------------- | ------------- | ----------- |
| `imm[11:0]`               | rs1                          | 000             | rd                              | 1100111       | JALR        |
| `imm[11:0]`               | rs1                          | 000             | rd                              | 0000011       | LB          |
| `imm[11:0]`               | rs1                          | 001             | rd                              | 0000011       | LH          |
| `imm[11:0]`               | rs1                          | 010             | rd                              | 0000011       | LW          |
| `imm[11:0]`               | rs1                          | 100             | rd                              | 0000011       | LBU         |
| `imm[11:0]`               | rs1                          | 101             | rd                              | 0000011       | LHU         |
| `imm[11:0]`               | rs1                          | 000             | rd                              | 0010011       | ADDI        |
| `imm[11:0]`               | rs1                          | 010             | rd                              | 0010011       | SLTI        |
| `imm[11:0]`               | rs1                          | 011             | rd                              | 0010011       | SLTIU       |
| `imm[11:0]`               | rs1                          | 100             | rd                              | 0010011       | XORI        |
| `imm[11:0]`               | rs1                          | 110             | rd                              | 0010011       | ORI         |
| `imm[11:0]`               | rs1                          | 111             | rd                              | 0010011       | ANDI        |

### S-Type


| Immediate`[31:25]` | Source Register rs2`[24:20]` | Source Register rs1`[19:15]` | funct3`[14:12]` | Immediate`[11:7]` | Opcode`[6:0]` | Instruction |
| ------------------ | ---------------------------- | ---------------------------- | --------------- | ----------------- | ------------- | ----------- |
| `imm[11:5]`        | rs2                          | rs1                          | 000             | `imm[4:0]`        | 0100011       | SB          |
| `imm[11:5]`        | rs2                          | rs1                          | 001             | `imm[4:0]`        | 0100011       | SH          |
| `imm[11:5]`        | rs2                          | rs1                          | 010             | `imm[4:0]`        | 0100011       | SW          |

### I-Type Shift Instructions


| funct7`[31:25]` | Shift Amount shamt`[24:20]` | Source Register rs1`[19:15]` | funct3`[14:12]` | Destination Register rd`[11:7]` | Opcode`[6:0]` | Instruction |
| --------------- | --------------------------- | ---------------------------- | --------------- | ------------------------------- | ------------- | ----------- |
| 0000000         | shamt                       | rs1                          | 001             | rd                              | 0010011       | SLLI        |
| 0000000         | shamt                       | rs1                          | 101             | rd                              | 0010011       | SRLI        |
| 0100000         | shamt                       | rs1                          | 101             | rd                              | 0010011       | SRAI        |

### R-Type Instructions


| funct7`[31:25]` | Source Register rs2`[24:20]` | Source Register rs1`[19:15]` | funct3`[14:12]` | Destination Register rd`[11:7]` | Opcode`[6:0]` | Instruction |
| --------------- | ---------------------------- | ---------------------------- | --------------- | ------------------------------- | ------------- | ----------- |
| 0000000         | rs2                          | rs1                          | 000             | rd                              | 0110011       | ADD         |
| 0100000         | rs2                          | rs1                          | 000             | rd                              | 0110011       | SUB         |
| 0000000         | rs2                          | rs1                          | 001             | rd                              | 0110011       | SLL         |
| 0000000         | rs2                          | rs1                          | 010             | rd                              | 0110011       | SLT         |
| 0000000         | rs2                          | rs1                          | 011             | rd                              | 0110011       | SLTU        |
| 0000000         | rs2                          | rs1                          | 100             | rd                              | 0110011       | XOR         |
| 0000000         | rs2                          | rs1                          | 101             | rd                              | 0110011       | SRL         |
| 0100000         | rs2                          | rs1                          | 101             | rd                              | 0110011       | SRA         |
| 0000000         | rs2                          | rs1                          | 110             | rd                              | 0110011       | OR          |
| 0000000         | rs2                          | rs1                          | 111             | rd                              | 0110011       | AND         |

### B-Type Instructions


| Branch Offset bit`[12]` `[31]` | Branch Offset bits`[10:5]` `[30:25]` | Source Register rs2`[24:20]` | Source Register rs1`[19:15]` | funct3`[14:12]` | Branch Offset bits`[4:1]` `[11:8]` | Branch Offset bit`[11]` `[7]` | Opcode`[6:0]` | Instruction |
| ------------------------------ | ------------------------------------ | ---------------------------- | ---------------------------- | --------------- | ---------------------------------- | ----------------------------- | ------------- | ----------- |
| `imm[12]`                      | `imm[10:5]`                          | rs2                          | rs1                          | 000             | `imm[4:1]`                         | `imm[11]`                     | 1100011       | BEQ         |
| `imm[12]`                      | `imm[10:5]`                          | rs2                          | rs1                          | 001             | `imm[4:1]`                         | `imm[11]`                     | 1100011       | BNE         |
| `imm[12]`                      | `imm[10:5]`                          | rs2                          | rs1                          | 100             | `imm[4:1]`                         | `imm[11]`                     | 1100011       | BLT         |
| `imm[12]`                      | `imm[10:5]`                          | rs2                          | rs1                          | 101             | `imm[4:1]`                         | `imm[11]`                     | 1100011       | BGE         |
| `imm[12]`                      | `imm[10:5]`                          | rs2                          | rs1                          | 110             | `imm[4:1]`                         | `imm[11]`                     | 1100011       | BLTU        |
| `imm[12]`                      | `imm[10:5]`                          | rs2                          | rs1                          | 111             | `imm[4:1]`                         | `imm[11]`                     | 1100011       | BGEU        |

### J-Type Instructions:


| Jump Offset bit`[20]` `[31]` | Jump Offset bits`[10:1]` `[30:21]` | Jump Offset bit`[11]` `[20]` | Jump Offset bits`[19:12]` `[19:12]` | Destination Register rd`[11:7]` | Opcode`[6:0]` | Instruction |
| ---------------------------- | ---------------------------------- | ---------------------------- | ----------------------------------- | ------------------------------- | ------------- | ----------- |
| `imm[20]`                    | `imm[10:1]`                        | `imm[11]`                    | `imm[19:12]`                        | rd                              | 1101111       | JAL         |

### Fence-Type Instructions


| Fence Mode fm`[31:28]` | Predecessor pred`[27:24]` | Successor succ`[23:20]` | Source Register rs1`[19:15]` | funct3`[14:12]` | Destination Register rd`[11:7]` | Opcode`[6:0]` | Instruction |
| ---------------------- | ------------------------- | ----------------------- | ---------------------------- | --------------- | ------------------------------- | ------------- | ----------- |
| 0000                   | 0000                      | 0000                    | 00000                        | 000             | rd                              | 0001111       | FENCE       |
| 1000                   | 0011                      | 0011                    | 00000                        | 000             | 00000                           | 0001111       | FENCE.TSO   |
| 0000                   | 0001                      | 0000                    | 00000                        | 000             | 00000                           | 0001111       | PAUSE       |

### System Instructions (Environment Call/Break)


| Zero Padding / Code`[31:20]` | Source Register rs1`[19:15]` | funct3`[14:12]` | Destination Register rd`[11:7]` | Opcode`[6:0]` | Instruction |
| ---------------------------- | ---------------------------- | --------------- | ------------------------------- | ------------- | ----------- |
| 000000000000                 | 00000                        | 000             | 00000                           | 1110011       | ECALL       |
| 000000000001                 | 00000                        | 000             | 00000                           | 1110011       | EBREAK      |
