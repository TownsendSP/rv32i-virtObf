// dis_rv32i.cpp
#include "dis_rv32i.h"
#include <stdexcept>
#include <sstream>

std::unique_ptr<Instruction> Instruction::create(uint32_t raw) {
    switch (raw & 0x7F) {
        case 0b1100111:
        case 0b0000011:
        case 0b0010011:
            return std::make_unique<IType>(raw);
        case 0b0100011:
            return std::make_unique<SType>(raw);
        case 0b0110011:
            return std::make_unique<RType>(raw);
        case 0b1100011:
            return std::make_unique<BType>(raw);
        case 0b0110111:
        case 0b0010111:
            return std::make_unique<UType>(raw);
        case 0b1101111:
            return std::make_unique<JType>(raw);
        case 0b0001111:
            return std::make_unique<FenceType>(raw);
        case 0b1110011:
            return std::make_unique<SysType>(raw);
        default:
            throw std::invalid_argument("Unknown opcode");
    }
}

// ------------------ IType ------------------
IType::IType(uint32_t raw)
    : Instruction(raw)
      , imm(static_cast<int32_t>(raw) >> 20) // bits [31:20], sign‑extend
      , rs1(static_cast<uint8_t>((raw >> 15) & 0x1F))
      , funct3(static_cast<uint8_t>((raw >> 12) & 0x07))
      , rd(static_cast<uint8_t>((raw >> 7) & 0x1F)) {
    switch (opcode) {
        case 0b0010011: // arithmetic immediates
            switch (funct3) {
                case 0: mnemonic = ADDI;
                    break;
                case 0b010: mnemonic = SLTI;
                    break;
                case 0b011: mnemonic = SLTIU;
                    break;
                case 0b100: mnemonic = XORI;
                    break;
                case 0b110: mnemonic = ORI;
                    break;
                case 0b111: mnemonic = ANDI;
                    break;
                case 0b001: mnemonic = SLLI;
                    break;
                case 0b101:
                    mnemonic = ((raw >> 30) & 1) ? SRAI : SRLI;
                    break;
                default: throw std::invalid_argument("Unknown I‑type funct3");
            }
            break;

        case 0b0000011: // loads
            switch (funct3) {
                case 0: mnemonic = LB;
                    break;
                case 1: mnemonic = LH;
                    break;
                case 2: mnemonic = LW;
                    break;
                case 4: mnemonic = LBU;
                    break;
                case 5: mnemonic = LHU;
                    break;
                default: throw std::invalid_argument("Unknown load funct3");
            }
            break;

        case 0b1100111: // JALR
            mnemonic = JALR;
            break;

        default:
            throw std::invalid_argument("Opcode not I‑type in IType ctor");
    }
}

std::string IType::toString() const {
    std::ostringstream os;
    // use our enum→string helper:
    os << mnemonicToString(mnemonic)
            << " x" << int(rd)
            << ", x" << int(rs1)
            << ", " << imm;
    return os.str();
}

// ------------------ UType ------------------
UType::UType(uint32_t raw)
    : Instruction(raw)
      , imm(static_cast<uint32_t>(raw) & 0xFFFFF000) // bits [31:12]
      , rd(static_cast<uint8_t>((raw >> 7) & 0x1F)) {
    switch (opcode) {
        case 0b0110111: mnemonic = LUI;
            break;
        case 0b0010111: mnemonic = AUIPC;
            break;
        default:
            throw std::invalid_argument("Opcode not U‑type in UType ctor: received opcode " + std::to_string(opcode));
    }
}

std::string UType::toString() const {
    std::ostringstream os;
    // use our enum→string helper:
    os << mnemonicToString(mnemonic)
            << " x" << int(rd)
            << ", " << imm;
    return os.str();
}

// ------------------ SType ------------------

SType::SType(uint32_t raw)
    : Instruction(raw) {
    // split the two halves of the 12‑bit imm
    uint32_t hi5 = (raw >> 25) & 0x7F; // imm[11:5]
    uint32_t lo5 = (raw >> 7) & 0x1F; // imm[4:0]
    uint32_t imm12 = (hi5 << 5) | lo5; // combine into [11:0]

    // sign‑extend to 32 bits
    if (imm12 & 0x800)
        imm = static_cast<int32_t>(imm12 | 0xFFFFF000);
    else
        imm = static_cast<int32_t>(imm12);

    // now the registers and funct3
    rs1 = static_cast<uint8_t>((raw >> 15) & 0x1F);
    rs2 = static_cast<uint8_t>((raw >> 20) & 0x1F);
    funct3 = static_cast<uint8_t>((raw >> 12) & 0x07);

    // pick the mnemonic
    switch (funct3) {
        case 0b000: mnemonic = SB;
            break;
        case 0b001: mnemonic = SH;
            break;
        case 0b010: mnemonic = SW;
            break;
        default: throw std::invalid_argument("Unknown S‑type funct3");
    }
}

std::string SType::toString() const {
    std::ostringstream os;
    os << mnemonicToString(mnemonic) << " x" << int(rs2)
            << ", " << imm
            << "(x" << int(rs1) << ")";
}

// ------------------ RType ------------------
RType::RType(uint32_t raw)
    : Instruction(raw) {
    // Extract fields from the raw instruction
    funct7 = static_cast<uint8_t>((raw >> 25) & 0x7F); // bits [31:25]
    rs2 = static_cast<uint8_t>((raw >> 20) & 0x1F); // bits [24:20]
    rs1 = static_cast<uint8_t>((raw >> 15) & 0x1F); // bits [19:15]
    funct3 = static_cast<uint8_t>((raw >> 12) & 0x07); // bits [14:12]
    rd = static_cast<uint8_t>((raw >> 7) & 0x1F); // bits [11:7]

    // Determine the mnemonic based on funct3 and funct7
    switch (funct3) {
        case 0b000:
            mnemonic = (funct7 == 0b0000000)
                           ? ADD
                           : (funct7 == 0b0100000)
                                 ? SUB
                                 : throw std::invalid_argument("Unknown R‑type funct7 for funct3=000");
            break;
        case 0b001: mnemonic = SLL;
            break;
        case 0b010: mnemonic = SLT;
            break;
        case 0b011: mnemonic = SLTU;
            break;
        case 0b100: mnemonic = XOR;
            break;
        case 0b101:
            mnemonic = (funct7 == 0b0000000)
                           ? SRL
                           : (funct7 == 0b0100000)
                                 ? SRA
                                 : throw std::invalid_argument("Unknown R‑type funct7 for funct3=101");
            break;
        case 0b110: mnemonic = OR;
            break;
        case 0b111: mnemonic = AND;
            break;
        default:
            throw std::invalid_argument("Unknown R‑type funct3");
    }
}

std::string RType::toString() const {
    std::ostringstream os;
    os << mnemonicToString(mnemonic)
            << " x" << int(rd)
            << ", x" << int(rs1)
            << ", x" << int(rs2);
    return os.str();
}

// ------------------ BType ------------------


// BType constructor
BType::BType(uint32_t raw)
    : Instruction(raw) {
    // extract branch-immediate parts
    uint32_t b12 = (raw >> 31) & 0b1; // imm[12]
    uint32_t b11 = (raw >> 7) & 0b1; // imm[11]
    uint32_t b10_5 = (raw >> 25) & 0b111111; // imm[10:5]
    uint32_t b4_1 = (raw >> 8) & 0b1111; // imm[4:1]

    // combine into full 13-bit immediate (bit0 is always zero)
    uint32_t imm13 = (b12 << 12)
                     | (b11 << 11)
                     | (b10_5 << 5)
                     | (b4_1 << 1);

    // sign-extend from bit 12
    if (imm13 & 0b1000000000000)
        imm = static_cast<int32_t>(imm13 | 0b11111111111111110000000000000000);
    else
        imm = static_cast<int32_t>(imm13);

    // extract registers & funct3
    rs1 = static_cast<uint8_t>((raw >> 15) & 0b11111); // bits [19:15]
    rs2 = static_cast<uint8_t>((raw >> 20) & 0b11111); // bits [24:20]
    funct3 = static_cast<uint8_t>((raw >> 12) & 0b111); // bits [14:12]

    // select branch mnemonic
    switch (funct3) {
        case 0b000: mnemonic = BEQ;
            break;
        case 0b001: mnemonic = BNE;
            break;
        case 0b100: mnemonic = BLT;
            break;
        case 0b101: mnemonic = wBGE;
            break;
        case 0b110: mnemonic = BLTU;
            break;
        case 0b111: mnemonic = BGEU;
            break;
        default: throw std::invalid_argument("Unknown B-type funct3");
    }
}

std::string BType::toString() const {
    std::ostringstream os;
    os << mnemonicToString(mnemonic)
            << " x" << int(rs1)
            << ", x" << int(rs2)
            << ", " << imm;
    return os.str();;
}

// ------------------ JType ------------------

JType::JType(uint32_t raw)
    : Instruction(raw) {
    // extract and sign-extend contiguous 20-bit immediate from bits [31:12]
    // mask = binary 11111111111111111111000000000000
    imm = static_cast<int32_t>(raw & 0b11111111111111111111000000000000) >> 11;

    // extract destination register
    rd = static_cast<uint8_t>((raw >> 7) & 0b11111); // bits [11:7]

    // select jump mnemonic
    mnemonic = JAL;
}

std::string JType::toString() const {
    std::ostringstream os;
    os << mnemonicToString(mnemonic)
            << " x" << int(rd)
            << ", " << imm;
    return os.str();
}

// ------------------ FenceType ------------------
// FenceType constructor
FenceType::FenceType(uint32_t raw)
  : Instruction(raw)
{
    fm     = static_cast<uint8_t>((raw >> 28) & 0b1111);   // bits [31:28]
    pred   = static_cast<uint8_t>((raw >> 24) & 0b1111);   // bits [27:24]
    succ   = static_cast<uint8_t>((raw >> 20) & 0b1111);   // bits [23:20]
    rs1    = static_cast<uint8_t>((raw >> 15) & 0b11111);  // bits [19:15]
    funct3 = static_cast<uint8_t>((raw >> 12) & 0b111);    // bits [14:12]
    rd     = static_cast<uint8_t>((raw >>  7) & 0b11111);  // bits [11:7]

    // funct3 for fence variants must be 000
    if (funct3 != 0b000)
        throw std::invalid_argument("Invalid fence funct3");

    // decide exact fence variant
    if      (fm == 0b0000 && pred == 0b0000 && succ == 0b0000)
        mnemonic = FENCE;
    else if (fm == 0b1000 && pred == 0b0011 && succ == 0b0011)
        mnemonic = FENCE_TSO;
    else if (fm == 0b0000 && pred == 0b0001 && succ == 0b0000)
        mnemonic = PAUSE;
    else
        throw std::invalid_argument("Unknown fence variant");
}

std::string FenceType::toString() const {
    std::ostringstream os;
    os << mnemonicToString(mnemonic)
            << " " << fm
            << ", " << pred
            << ", " << succ;
    return os.str();
}

// ------------------ SysType ------------------


SysType::SysType(uint32_t raw)
  : Instruction(raw)
{
    uint32_t code = (raw >> 20) & 0b111111111111; // bits [31:20]
    rs1    = static_cast<uint8_t>((raw >> 15) & 0b11111);
    funct3 = static_cast<uint8_t>((raw >> 12) & 0b111);
    rd     = static_cast<uint8_t>((raw >>  7) & 0b11111);

    if (rs1 != 0b00000 || funct3 != 0b000 || rd != 0b00000)
        throw std::invalid_argument("Invalid system instruction format");

    switch (code) {
        case 0b000000000000: mnemonic = ECALL;  break;
        case 0b000000000001: mnemonic = EBREAK; break;
        default: throw std::invalid_argument("Unknown system code");
    }
}

std::string SysType::toString() const {
    std::ostringstream os;
    os << mnemonicToString(mnemonic);
    return os.str();
}