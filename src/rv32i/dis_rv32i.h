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
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
    BEQ, BNE, BLT, BGE, BLTU, BGEU,
    JAL,
    RET,
    FENCE, FENCE_TSO, PAUSE,
    ECALL, EBREAK
};


// ─── helper to go from enum→string ────────────────────────────────────────────
inline const char *mnemonicToString(MNEMONIC m) {
    static constexpr const char *names[] = {
        "LUI", "AUIPC",
        "JALR", "LB", "LH", "LW", "LBU", "LHU", "ADDI", "SLTI", "SLTIU", "XORI", "ORI", "ANDI",
        "SB", "SH", "SW",
        "SLLI", "SRLI", "SRAI",
        "ADD", "SUB", "SLL", "SLT", "SLTU", "XOR", "SRL", "SRA", "OR", "AND",
        "BEQ", "BNE", "BLT", "BGE", "BLTU", "BGEU",
        "JAL",
        "RET",
        "FENCE", "FENCE_TSO", "PAUSE",
        "ECALL", "EBREAK"
    };
    return names[static_cast<size_t>(m)];
}

class Instruction {
public:
    // Factory: returns the correct subclass based on the low‑7 bits
    static std::unique_ptr<Instruction> create(uint32_t raw);

    virtual ~Instruction() = default;

    // Must be overridden by every derived class
    virtual std::string toString() const = 0;

    // Getters for instruction analysis
    uint32_t getRaw() const { return raw; }
    uint8_t getOpcode() const { return opcode; }
    MNEMONIC getMnemonic() const { return mnemonic; }

    // Methods for control flow analysis
    virtual bool isBranch() const { return false; }
    virtual bool isJump() const { return false; }
    virtual bool isConditional() const { return false; }
    virtual int32_t getImmediate() const { return 0; }  // For branch/jump targets

protected:
    explicit Instruction(uint32_t raw)
        : raw(raw), opcode(static_cast<uint8_t>(raw & 0x7F)) {
    }

    uint32_t raw; //< the full 32‑bit instruction
    uint8_t opcode; //< bits[6:0]
    MNEMONIC mnemonic; // enum goes here
};

// I‑type: imm[31:20] | rs1[19:15] | funct3[14:12] | rd[11:7] | opcode[6:0]
class IType : public Instruction {
public:
    explicit IType(uint32_t raw);

    std::string toString() const override;

    bool isJump() const override { return mnemonic == JALR; }
    int32_t getImmediate() const override { return imm; }

    int32_t imm; //< sign‑extended 12‑bit immediate
    uint8_t rs1, funct3, rd;

private:
};

// U‑type: imm[31:12] | rd[11:7] | opcode[6:0]
class UType : public Instruction {
public:
    explicit UType(uint32_t raw);

    std::string toString() const override;

    uint32_t imm; // 31:12
    uint8_t rd; // 11:7

private:
};

// S‑type: imm[31:25] | rs2[24:20] | rs1[19:15] | funct3[14:12] | imm[11:7] | opcode[6:0]
class SType : public Instruction {
public:
    explicit SType(uint32_t raw);

    std::string toString() const override;

    int32_t imm; // 31:25 and 11:7
    uint8_t rs1, rs2, funct3;

private:
};

// R‑type: funct7[31:25] | rs2[24:20] | rs1[19:15] | funct3[14:12] | rd[11:7] | opcode[6:0]
class RType : public Instruction {
public:
    explicit RType(uint32_t raw);

    std::string toString() const override;

    uint8_t funct7; // Function code (bits 31–25)
    uint8_t rs2; // Source register 2 (bits 24–20)
    uint8_t rs1; // Source register 1 (bits 19–15)
    uint8_t funct3; // Function code (bits 14–12)
    uint8_t rd; // Destination register (bits 11–7)

private:
};

// B‑type: imm[31:25] | rs2[24:20] | rs1[19:15] | funct3[14:12] | imm[11:7] | opcode[6:0]
class BType : public Instruction {
public:
    explicit BType(uint32_t raw);

    std::string toString() const override;

    bool isBranch() const override { return true; }
    bool isConditional() const override { return true; }
    int32_t getImmediate() const override { return imm; }

    int32_t imm; ///< sign‑extended 12‑bit immediate
    uint8_t rs1, rs2, funct3;

private:
};


// J‑type: imm[31:12] | rd[11:7] | opcode[6:0]
class JType : public Instruction {
public:
    //JType(uint32_t raw) : Instruction(raw) {};
    std::string toString() const override;

    explicit JType(uint32_t raw);

    bool isJump() const override { return true; }
    int32_t getImmediate() const override { return imm; }

    int32_t imm;   // Jump target offset (sign-extended 21-bit immediate)
    uint8_t rd; // Destination register (bits 11–7)

private:
};

// FenceType: fence[31:28] | predicate[27:24] | succ[23:20] | rs1[19:15] | funct3[14:12] | rd[11:7] | opcode[6:0]
class FenceType : public Instruction {
public:
    //FenceType(uint32_t raw) : Instruction(raw) {};
    std::string toString() const override;

    explicit FenceType(uint32_t raw);

    uint8_t fm; // Fence mode (bits 31–28)
    uint8_t pred; // Predicate (bits 27–24)
    uint8_t succ; // Successor (bits 23–20)
    uint8_t rs1; // Source register 1 (bits 19–15)  - always all 0s
    uint8_t funct3; // Function code (bits 14–12) - always 000
    uint8_t rd; // Destination register (bits 11–7) - always all 0s

private:
};


// SysType: zeroPadding[31:20] | rs1[19:15] | funct3[14:12] | rd[11:7] | opcode[6:0]
class SysType : public Instruction {
public:
    //SysType(uint32_t raw) : Instruction(raw) {};
    std::string toString() const override;

    explicit SysType(uint32_t raw);

    uint16_t zeroPadding; // Zero padding (bits 31–20)
    uint8_t rs1; // Source register 1 (bits 19–15) - always all 0s
    uint8_t funct3; // Function code (bits 14–12) - always 000
    uint8_t rd; // Destination register (bits 11–7) - always all 0s

private:
};

std::unique_ptr<Instruction> decodeInstruction(uint32_t rawInst);

#endif // DIS_RV32I_H
