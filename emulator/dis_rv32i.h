// dis_rv32i.h
#ifndef DIS_RV32I_H
#define DIS_RV32I_H

#include <cstdint>
#include <string>
#include <memory>

// enum INS_TYPE { U_TYPE, I_TYPE, S_TYPE, RSHIFT_TYPE, R_TYPE, B_TYPE, J_TYPE, FENCE_TYPE, SYS_TYPE };
enum INS_TYPE { U_TYPE, I_TYPE, S_TYPE, R_TYPE, B_TYPE, J_TYPE, FENCE_TYPE, SYS_TYPE };

enum MNEMONIC {
    LUI, AUIPC,
    JALR, LB, LH, LW, LBU, LHU, ADDI, SLTI, SLTIU, XORI, ORI, ANDI,
    SB, SH, SW,
    SLLI, SRLI, SRAI,
    ADD,SUB,SLL,SLT,SLTU,XOR,SRL,SRA,OR,AND,
    BEQ, BNE, BLT, BGE, BLTU, BGEU,
    JAL,
    FENCE, FENCE_TSO, PAUSE,
    ECALL, EBREAK
};

class Instruction {
    INS_TYPE type;
    uint32_t raw;
    uint8_t opcode;
    MNEMONIC mnemonic;

public:
    Instruction(std::string inputhex);

    Instruction(uint32_t raw);

    std::unique_ptr<Instruction> create(uint32_t raw);

private:
    virtual std::string toString() const = 0;
    virtual ~Instruction() = default;
};

class UType : public Instruction {

public:
    uint32_t imm; // 31:12
    uint8_t rd;   // 11:7

    UType(uint32_t raw) : Instruction(raw) {}


    std::string toString() const override;
};

class IType : public Instruction {

public:
    uint16_t imm; // Immediate value (12-bit immediate)
    uint8_t rs1;  // Source register 1 (bits 19–15)
    uint8_t funct3; // Function code (bits 14–12)
    uint8_t rd;   // Destination register (bits 11–7)

    //IType(uint32_t raw) : Instruction(raw) {};

    std::string toString() const override;

    IType(uint32_t raw);
};

class SType : public Instruction {
public:
    uint16_t imm; // Immediate value (12-bit immediate) 31:25
    uint8_t rs2; // Source register 2 (bits 24–20)
    uint8_t rs1; // Source register 1 (bits 19–15)
    uint8_t funct3; // Function code (bits 14–12)
    uint8_t imm2; // Immediate value (12-bit immediate) 11:7

 //SType(uint32_t raw) : Instruction(raw) {};
    std::string toString() const override;

    SType(uint32_t raw);
};

// class RTypeShift : public Instruction {
// public:
//     uint8_t funct7; // Function code (bits 31–25)
//     uint8_t shamt; // Shift amount (bits 24–20)
//     uint8_t rs1;   // Source register 1 (bits 19–15)
//     uint8_t funct3; // Function code (bits 14–12)
//     uint8_t rd;    // Destination register (bits 11–7)
//
//     std::string toString() const override;
// };

class RType : public Instruction {
public:
    uint8_t funct7; // Function code (bits 31–25)
    uint8_t rs2;  // Source register 2 (bits 24–20)
    uint8_t rs1;   // Source register 1 (bits 19–15)
    uint8_t funct3; // Function code (bits 14–12)
    uint8_t rd;    // Destination register (bits 11–7)
    //RType(uint32_t raw) : Instruction(raw) {};
    std::string toString() const override;

    RType(uint32_t raw);
};

class BType : public Instruction {
public:
    uint8_t imm_branch_offset; // Branch target offset (12-bit immediate, encoded non-contiguously) imm[12|10:5]
    uint8_t rs2; // Source register 2 (bits 24–20)
    uint8_t rs1; // Source register 1 (bits 19–15)
    uint8_t funct3; // Function code (bits 14–12)
    uint8_t imm2; // Branch target offset (12-bit immediate, encoded non-contiguously) imm[4:1|11]

 //BType(uint32_t raw) : Instruction(raw) {};
    std::string toString() const override;

    BType(uint32_t raw);
};

class JType : public Instruction {
public:
    uint32_t imm;  // Jump target offset (20-bit immediate, encoded non-contiguously)
    uint8_t rd;    // Destination register (bits 11–7)

 //JType(uint32_t raw) : Instruction(raw) {};
    std::string toString() const override;

    JType(uint32_t raw);
};

class FenceType : public Instruction {
public:
    uint8_t fenceMode; // Fence mode (bits 31–28)
    uint8_t predicate; // Predicate (bits 27–24)
    uint8_t succ; // Successor (bits 23–20)
    uint8_t rs1; // Source register 1 (bits 19–15)  - always all 0s
    uint8_t funct3; // Function code (bits 14–12) - always 000
    uint8_t rd; // Destination register (bits 11–7) - always all 0s

 //FenceType(uint32_t raw) : Instruction(raw) {};
    std::string toString() const override;

    FenceType(uint32_t raw);
};

class SysType : public Instruction {
    uint16_t zeroPadding; // Zero padding (bits 31–20)
    uint8_t rs1; // Source register 1 (bits 19–15) - always all 0s
    uint8_t funct3; // Function code (bits 14–12) - always 000
    uint8_t rd; // Destination register (bits 11–7) - always all 0s

 //SysType(uint32_t raw) : Instruction(raw) {};
    std::string toString() const override;

    SysType(uint32_t raw);
};

// Factory method to create proper Instruction type from raw
std::unique_ptr<Instruction> decodeInstruction(uint32_t rawInst);

#endif // DIS_RV32I_H
