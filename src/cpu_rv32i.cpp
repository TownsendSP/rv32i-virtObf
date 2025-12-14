#include "cpu_rv32i.h"

cpu_rv32i::cpu_rv32i(): pc(0) {
    // Initialize all registers to 0
    for (int i = 0; i < 32; i++) {
        registers[i] = 0;
    }
    // Set stack pointer to top of stack
    registers[2] = memory.get_stack_ptr();  // sp = x2
}

void cpu_rv32i::load_program(const std::vector<uint8_t> &program) {
    memory.load_code(program);
    pc = memory.get_code_base();  // Start execution at beginning of code
}

uint32_t cpu_rv32i::read_reg(uint8_t reg) const {
    if (reg == 0) return 0;  // x0 is always 0
    return registers[reg];
}

void cpu_rv32i::write_reg(uint8_t reg, uint32_t value) {
    if (reg == 0) return;  // Writing to x0 is ignored
    registers[reg] = value;
}

uint32_t cpu_rv32i::fetch() {
    uint32_t inst = memory.read32(pc);
    return inst;
}

void cpu_rv32i::advance_pc() {
    pc += 4;
}

void cpu_rv32i::branch(int32_t offset) {
    pc += offset;
}

void cpu_rv32i::jump(uint32_t target) {
    pc = target;
}
template <typename T>
T* as(const std::unique_ptr<Instruction>& inst) {
    return static_cast<T*>(inst.get());
}

void cpu_rv32i::execute(const std::vector<std::unique_ptr<Instruction>>& instructions) {
    uint32_t code_base = memory.get_code_base();

    while (true) {
        // 1. Fetch logic (virtual)
        if (pc < code_base) {
            throw std::runtime_error("PC out of bounds (underflow)");
        }
        uint32_t offset = pc - code_base;
        if (offset % 4 != 0) {
            throw std::runtime_error("PC alignment error");
        }
        size_t index = offset / 4;

        if (index >= instructions.size()) {
            throw std::runtime_error("PC out of bounds (overflow)");
        }

        const auto& inst = instructions[index];
        MNEMONIC m = inst->getMnemonic();

        // Default next PC
        uint32_t next_pc = pc + 4;
        bool branch_taken = false;

        switch (m) {
            // ---------------- U-Type ----------------
            case LUI: { // Load Upper Immediate
                auto* i = as<UType>(inst);
                write_reg(i->rd, i->imm);
                break;
            }
            case AUIPC: { // Add Upper Immediate to PC
                auto* i = as<UType>(inst);
                write_reg(i->rd, pc + i->imm);
                break;
            }

            // ---------------- J-Type ----------------
            case JAL: { // Jump and Link
                auto* i = as<JType>(inst);
                write_reg(i->rd, pc + 4);
                next_pc = pc + i->imm;
                branch_taken = true;
                break;
            }

            // ---------------- I-Type (Jumps) ----------------
            case JALR: { // Jump and Link Register
                auto* i = as<IType>(inst);
                uint32_t target = read_reg(i->rs1) + i->imm;
                target &= ~1; // Clear LSB
                write_reg(i->rd, pc + 4);
                next_pc = target;
                branch_taken = true;
                break;
            }
            case RET: { // Pseudo-instruction for JALR x0, x1, 0
                // Stop execution and return
                return;
            }

            // ---------------- B-Type (Branches) ----------------
            case BEQ: {
                auto* i = as<BType>(inst);
                if (read_reg(i->rs1) == read_reg(i->rs2)) {
                    next_pc = pc + i->imm;
                    branch_taken = true;
                }
                break;
            }
            case BNE: {
                auto* i = as<BType>(inst);
                if (read_reg(i->rs1) != read_reg(i->rs2)) {
                    next_pc = pc + i->imm;
                    branch_taken = true;
                }
                break;
            }
            case BLT: {
                auto* i = as<BType>(inst);
                if ((int32_t)read_reg(i->rs1) < (int32_t)read_reg(i->rs2)) {
                    next_pc = pc + i->imm;
                    branch_taken = true;
                }
                break;
            }
            case BGE: {
                auto* i = as<BType>(inst);
                if ((int32_t)read_reg(i->rs1) >= (int32_t)read_reg(i->rs2)) {
                    next_pc = pc + i->imm;
                    branch_taken = true;
                }
                break;
            }
            case BLTU: {
                auto* i = as<BType>(inst);
                if (read_reg(i->rs1) < read_reg(i->rs2)) {
                    next_pc = pc + i->imm;
                    branch_taken = true;
                }
                break;
            }
            case BGEU: {
                auto* i = as<BType>(inst);
                if (read_reg(i->rs1) >= read_reg(i->rs2)) {
                    next_pc = pc + i->imm;
                    branch_taken = true;
                }
                break;
            }

            // ---------------- I-Type (Loads) ----------------
            case LB: {
                auto* i = as<IType>(inst);
                uint32_t addr = read_reg(i->rs1) + i->imm;
                int8_t val = (int8_t)memory.read8(addr);
                write_reg(i->rd, (int32_t)val);
                break;
            }
            case LH: {
                auto* i = as<IType>(inst);
                uint32_t addr = read_reg(i->rs1) + i->imm;
                int16_t val = (int16_t)memory.read16(addr);
                write_reg(i->rd, (int32_t)val);
                break;
            }
            case LW: {
                auto* i = as<IType>(inst);
                uint32_t addr = read_reg(i->rs1) + i->imm;
                uint32_t val = memory.read32(addr);
                write_reg(i->rd, val);
                break;
            }
            case LBU: {
                auto* i = as<IType>(inst);
                uint32_t addr = read_reg(i->rs1) + i->imm;
                uint8_t val = memory.read8(addr);
                write_reg(i->rd, val);
                break;
            }
            case LHU: {
                auto* i = as<IType>(inst);
                uint32_t addr = read_reg(i->rs1) + i->imm;
                uint16_t val = memory.read16(addr);
                write_reg(i->rd, val);
                break;
            }

            // ---------------- S-Type (Stores) ----------------
            case SB: {
                auto* i = as<SType>(inst);
                uint32_t addr = read_reg(i->rs1) + i->imm;
                memory.write8(addr, (uint8_t)read_reg(i->rs2));
                break;
            }
            case SH: {
                auto* i = as<SType>(inst);
                uint32_t addr = read_reg(i->rs1) + i->imm;
                memory.write16(addr, (uint16_t)read_reg(i->rs2));
                break;
            }
            case SW: {
                auto* i = as<SType>(inst);
                uint32_t addr = read_reg(i->rs1) + i->imm;
                memory.write32(addr, read_reg(i->rs2));
                break;
            }

            // ---------------- I-Type (ALU Immediates) ----------------
            case ADDI: {
                auto* i = as<IType>(inst);
                write_reg(i->rd, read_reg(i->rs1) + i->imm);
                break;
            }
            case SLTI: {
                auto* i = as<IType>(inst);
                write_reg(i->rd, ((int32_t)read_reg(i->rs1) < i->imm) ? 1 : 0);
                break;
            }
            case SLTIU: {
                auto* i = as<IType>(inst);
                write_reg(i->rd, (read_reg(i->rs1) < (uint32_t)i->imm) ? 1 : 0);
                break;
            }
            case XORI: {
                auto* i = as<IType>(inst);
                write_reg(i->rd, read_reg(i->rs1) ^ i->imm);
                break;
            }
            case ORI: {
                auto* i = as<IType>(inst);
                write_reg(i->rd, read_reg(i->rs1) | i->imm);
                break;
            }
            case ANDI: {
                auto* i = as<IType>(inst);
                write_reg(i->rd, read_reg(i->rs1) & i->imm);
                break;
            }
            case SLLI: {
                auto* i = as<IType>(inst);
                // shamt is lower 5 bits of imm
                uint32_t shamt = i->imm & 0x1F;
                write_reg(i->rd, read_reg(i->rs1) << shamt);
                break;
            }
            case SRLI: {
                auto* i = as<IType>(inst);
                uint32_t shamt = i->imm & 0x1F;
                write_reg(i->rd, read_reg(i->rs1) >> shamt);
                break;
            }
            case SRAI: {
                auto* i = as<IType>(inst);
                uint32_t shamt = i->imm & 0x1F;
                int32_t val = (int32_t)read_reg(i->rs1);
                write_reg(i->rd, (uint32_t)(val >> shamt));
                break;
            }

            // ---------------- R-Type (ALU Register) ----------------
            case ADD: {
                auto* i = as<RType>(inst);
                write_reg(i->rd, read_reg(i->rs1) + read_reg(i->rs2));
                break;
            }
            case SUB: {
                auto* i = as<RType>(inst);
                write_reg(i->rd, read_reg(i->rs1) - read_reg(i->rs2));
                break;
            }
            case SLL: {
                auto* i = as<RType>(inst);
                uint32_t shamt = read_reg(i->rs2) & 0x1F;
                write_reg(i->rd, read_reg(i->rs1) << shamt);
                break;
            }
            case SLT: {
                auto* i = as<RType>(inst);
                write_reg(i->rd, ((int32_t)read_reg(i->rs1) < (int32_t)read_reg(i->rs2)) ? 1 : 0);
                break;
            }
            case SLTU: {
                auto* i = as<RType>(inst);
                write_reg(i->rd, (read_reg(i->rs1) < read_reg(i->rs2)) ? 1 : 0);
                break;
            }
            case XOR: {
                auto* i = as<RType>(inst);
                write_reg(i->rd, read_reg(i->rs1) ^ read_reg(i->rs2));
                break;
            }
            case SRL: {
                auto* i = as<RType>(inst);
                uint32_t shamt = read_reg(i->rs2) & 0x1F;
                write_reg(i->rd, read_reg(i->rs1) >> shamt);
                break;
            }
            case SRA: {
                auto* i = as<RType>(inst);
                uint32_t shamt = read_reg(i->rs2) & 0x1F;
                int32_t val = (int32_t)read_reg(i->rs1);
                write_reg(i->rd, (uint32_t)(val >> shamt));
                break;
            }
            case OR: {
                auto* i = as<RType>(inst);
                write_reg(i->rd, read_reg(i->rs1) | read_reg(i->rs2));
                break;
            }
            case AND: {
                auto* i = as<RType>(inst);
                write_reg(i->rd, read_reg(i->rs1) & read_reg(i->rs2));
                break;
            }

            // ---------------- Misc ----------------
            case FENCE:
            case FENCE_TSO:
            case PAUSE:
                // No-op for now
                break;

            case ECALL:
            case EBREAK:
                throw std::runtime_error("ECALL/EBREAK not implemented");

            default:
                throw std::runtime_error("Unknown instruction mnemonic");
        }

        pc = next_pc;
    }
}

