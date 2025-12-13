#include "vmcore.h"
#include <iostream>

// Helper macro for safe dynamic_cast with null check
#define SAFE_CAST(type, ptr, mnemonic_str) \
    const type* ptr = dynamic_cast<const type*>(instr); \
    if (!ptr) throw std::runtime_error("Failed to cast " mnemonic_str " instruction");

vmcore::vmcore(): pc(0) {
    // Initialize VirtualMemory static layout
    VirtualMemory::init();
    
    // Initialize all registers to 0
    for (int i = 0; i < 32; i++) {
        registers[i] = 0;
    }
    // Set stack pointer to top of stack
    registers[2] = memory.get_stack_ptr();  // sp = x2
}

void vmcore::load_program(const std::vector<uint8_t> &program) {
    memory.load_code(program);
    pc = memory.get_code_base();  // Start execution at beginning of code
}

int32_t vmcore::execute(const std::vector<std::unique_ptr<Instruction>>& instructions, int32_t arg0, int32_t arg1) {
    // Set up arguments in registers a0 and a1 (x10 and x11)
    write_reg(10, static_cast<uint32_t>(arg0));
    write_reg(11, static_cast<uint32_t>(arg1));
    
    // Reset PC to 0 for instruction vector execution
    pc = 0;
    
    // Execute instructions until RET
    while (pc % 4 == 0 && pc / 4 < instructions.size()) {
        const Instruction* instr = instructions[pc / 4].get();
        
        // Check if this is a RET instruction
        if (instr->getMnemonic() == RET) {
            // Return value in a0 (x10)
            return static_cast<int32_t>(read_reg(10));
        }
        
        // Execute the instruction
        execute_instruction(instr);
    }
    
    // If we reach here without RET, return a0 anyway
    return static_cast<int32_t>(read_reg(10));
}

void vmcore::execute_instruction(const Instruction* instr) {
    if (!instr) {
        throw std::runtime_error("Null instruction pointer");
    }
    
    MNEMONIC mnem = instr->getMnemonic();
    
    switch (mnem) {
        // --- U-Type Instructions ---
        case LUI: {
            const UType* u = dynamic_cast<const UType*>(instr);
            if (!u) throw std::runtime_error("Failed to cast LUI instruction");
            write_reg(u->getRd(), u->getImm());
            pc += 4;
            break;
        }
        case AUIPC: {
            const UType* u = dynamic_cast<const UType*>(instr);
            if (!u) throw std::runtime_error("Failed to cast AUIPC instruction");
            write_reg(u->getRd(), pc + u->getImm());
            pc += 4;
            break;
        }
        
        // --- I-Type Arithmetic ---
        case ADDI: {
            const IType* i = dynamic_cast<const IType*>(instr);
            if (!i) throw std::runtime_error("Failed to cast ADDI instruction");
            write_reg(i->getRd(), read_reg(i->getRs1()) + i->getImmediate());
            pc += 4;
            break;
        }
        case SLTI: {
            const IType* i = dynamic_cast<const IType*>(instr);
            int32_t val = static_cast<int32_t>(read_reg(i->getRs1()));
            write_reg(i->getRd(), (val < i->getImmediate()) ? 1 : 0);
            pc += 4;
            break;
        }
        case SLTIU: {
            const IType* i = dynamic_cast<const IType*>(instr);
            uint32_t val = read_reg(i->getRs1());
            uint32_t imm = static_cast<uint32_t>(i->getImmediate());
            write_reg(i->getRd(), (val < imm) ? 1 : 0);
            pc += 4;
            break;
        }
        case XORI: {
            const IType* i = dynamic_cast<const IType*>(instr);
            write_reg(i->getRd(), read_reg(i->getRs1()) ^ i->getImmediate());
            pc += 4;
            break;
        }
        case ORI: {
            const IType* i = dynamic_cast<const IType*>(instr);
            write_reg(i->getRd(), read_reg(i->getRs1()) | i->getImmediate());
            pc += 4;
            break;
        }
        case ANDI: {
            const IType* i = dynamic_cast<const IType*>(instr);
            write_reg(i->getRd(), read_reg(i->getRs1()) & i->getImmediate());
            pc += 4;
            break;
        }
        case SLLI: {
            const IType* i = dynamic_cast<const IType*>(instr);
            uint32_t shamt = i->getImmediate() & 0x1F;  // Lower 5 bits
            write_reg(i->getRd(), read_reg(i->getRs1()) << shamt);
            pc += 4;
            break;
        }
        case SRLI: {
            const IType* i = dynamic_cast<const IType*>(instr);
            uint32_t shamt = i->getImmediate() & 0x1F;  // Lower 5 bits
            write_reg(i->getRd(), read_reg(i->getRs1()) >> shamt);
            pc += 4;
            break;
        }
        case SRAI: {
            const IType* i = dynamic_cast<const IType*>(instr);
            uint32_t shamt = i->getImmediate() & 0x1F;  // Lower 5 bits
            int32_t val = static_cast<int32_t>(read_reg(i->getRs1()));
            write_reg(i->getRd(), static_cast<uint32_t>(val >> shamt));
            pc += 4;
            break;
        }
        
        // --- I-Type Loads ---
        case LB: {
            const IType* i = dynamic_cast<const IType*>(instr);
            uint32_t addr = read_reg(i->getRs1()) + i->getImmediate();
            int8_t val = static_cast<int8_t>(memory.read8(addr));
            write_reg(i->getRd(), static_cast<uint32_t>(static_cast<int32_t>(val)));
            pc += 4;
            break;
        }
        case LH: {
            const IType* i = dynamic_cast<const IType*>(instr);
            uint32_t addr = read_reg(i->getRs1()) + i->getImmediate();
            int16_t val = static_cast<int16_t>(memory.read16(addr));
            write_reg(i->getRd(), static_cast<uint32_t>(static_cast<int32_t>(val)));
            pc += 4;
            break;
        }
        case LW: {
            const IType* i = dynamic_cast<const IType*>(instr);
            uint32_t addr = read_reg(i->getRs1()) + i->getImmediate();
            write_reg(i->getRd(), memory.read32(addr));
            pc += 4;
            break;
        }
        case LBU: {
            const IType* i = dynamic_cast<const IType*>(instr);
            uint32_t addr = read_reg(i->getRs1()) + i->getImmediate();
            write_reg(i->getRd(), memory.read8(addr));
            pc += 4;
            break;
        }
        case LHU: {
            const IType* i = dynamic_cast<const IType*>(instr);
            uint32_t addr = read_reg(i->getRs1()) + i->getImmediate();
            write_reg(i->getRd(), memory.read16(addr));
            pc += 4;
            break;
        }
        
        // --- S-Type Stores ---
        case SB: {
            const SType* s = dynamic_cast<const SType*>(instr);
            uint32_t addr = read_reg(s->getRs1()) + s->getImm();
            memory.write8(addr, static_cast<uint8_t>(read_reg(s->getRs2()) & 0xFF));
            pc += 4;
            break;
        }
        case SH: {
            const SType* s = dynamic_cast<const SType*>(instr);
            uint32_t addr = read_reg(s->getRs1()) + s->getImm();
            memory.write16(addr, static_cast<uint16_t>(read_reg(s->getRs2()) & 0xFFFF));
            pc += 4;
            break;
        }
        case SW: {
            const SType* s = dynamic_cast<const SType*>(instr);
            uint32_t addr = read_reg(s->getRs1()) + s->getImm();
            memory.write32(addr, read_reg(s->getRs2()));
            pc += 4;
            break;
        }
        
        // --- R-Type Instructions ---
        case ADD: {
            const RType* r = dynamic_cast<const RType*>(instr);
            write_reg(r->getRd(), read_reg(r->getRs1()) + read_reg(r->getRs2()));
            pc += 4;
            break;
        }
        case SUB: {
            const RType* r = dynamic_cast<const RType*>(instr);
            write_reg(r->getRd(), read_reg(r->getRs1()) - read_reg(r->getRs2()));
            pc += 4;
            break;
        }
        case SLL: {
            const RType* r = dynamic_cast<const RType*>(instr);
            uint32_t shamt = read_reg(r->getRs2()) & 0x1F;
            write_reg(r->getRd(), read_reg(r->getRs1()) << shamt);
            pc += 4;
            break;
        }
        case SLT: {
            const RType* r = dynamic_cast<const RType*>(instr);
            int32_t val1 = static_cast<int32_t>(read_reg(r->getRs1()));
            int32_t val2 = static_cast<int32_t>(read_reg(r->getRs2()));
            write_reg(r->getRd(), (val1 < val2) ? 1 : 0);
            pc += 4;
            break;
        }
        case SLTU: {
            const RType* r = dynamic_cast<const RType*>(instr);
            write_reg(r->getRd(), (read_reg(r->getRs1()) < read_reg(r->getRs2())) ? 1 : 0);
            pc += 4;
            break;
        }
        case XOR: {
            const RType* r = dynamic_cast<const RType*>(instr);
            write_reg(r->getRd(), read_reg(r->getRs1()) ^ read_reg(r->getRs2()));
            pc += 4;
            break;
        }
        case SRL: {
            const RType* r = dynamic_cast<const RType*>(instr);
            uint32_t shamt = read_reg(r->getRs2()) & 0x1F;
            write_reg(r->getRd(), read_reg(r->getRs1()) >> shamt);
            pc += 4;
            break;
        }
        case SRA: {
            const RType* r = dynamic_cast<const RType*>(instr);
            uint32_t shamt = read_reg(r->getRs2()) & 0x1F;
            int32_t val = static_cast<int32_t>(read_reg(r->getRs1()));
            write_reg(r->getRd(), static_cast<uint32_t>(val >> shamt));
            pc += 4;
            break;
        }
        case OR: {
            const RType* r = dynamic_cast<const RType*>(instr);
            write_reg(r->getRd(), read_reg(r->getRs1()) | read_reg(r->getRs2()));
            pc += 4;
            break;
        }
        case AND: {
            const RType* r = dynamic_cast<const RType*>(instr);
            write_reg(r->getRd(), read_reg(r->getRs1()) & read_reg(r->getRs2()));
            pc += 4;
            break;
        }
        
        // --- B-Type Branches ---
        case BEQ: {
            const BType* b = dynamic_cast<const BType*>(instr);
            if (read_reg(b->getRs1()) == read_reg(b->getRs2())) {
                pc += b->getImmediate();
            } else {
                pc += 4;
            }
            break;
        }
        case BNE: {
            const BType* b = dynamic_cast<const BType*>(instr);
            if (read_reg(b->getRs1()) != read_reg(b->getRs2())) {
                pc += b->getImmediate();
            } else {
                pc += 4;
            }
            break;
        }
        case BLT: {
            const BType* b = dynamic_cast<const BType*>(instr);
            int32_t val1 = static_cast<int32_t>(read_reg(b->getRs1()));
            int32_t val2 = static_cast<int32_t>(read_reg(b->getRs2()));
            if (val1 < val2) {
                pc += b->getImmediate();
            } else {
                pc += 4;
            }
            break;
        }
        case BGE: {
            const BType* b = dynamic_cast<const BType*>(instr);
            int32_t val1 = static_cast<int32_t>(read_reg(b->getRs1()));
            int32_t val2 = static_cast<int32_t>(read_reg(b->getRs2()));
            if (val1 >= val2) {
                pc += b->getImmediate();
            } else {
                pc += 4;
            }
            break;
        }
        case BLTU: {
            const BType* b = dynamic_cast<const BType*>(instr);
            if (read_reg(b->getRs1()) < read_reg(b->getRs2())) {
                pc += b->getImmediate();
            } else {
                pc += 4;
            }
            break;
        }
        case BGEU: {
            const BType* b = dynamic_cast<const BType*>(instr);
            if (read_reg(b->getRs1()) >= read_reg(b->getRs2())) {
                pc += b->getImmediate();
            } else {
                pc += 4;
            }
            break;
        }
        
        // --- J-Type Jumps ---
        case JAL: {
            const JType* j = dynamic_cast<const JType*>(instr);
            write_reg(j->getRd(), pc + 4);  // Save return address
            pc += j->getImmediate();
            break;
        }
        case JALR: {
            const IType* i = dynamic_cast<const IType*>(instr);
            if (!i) throw std::runtime_error("Failed to cast JALR instruction");
            // Clear LSB to ensure even alignment (RISC-V requirement)
            uint32_t target = (read_reg(i->getRs1()) + i->getImmediate()) & ~1u;
            write_reg(i->getRd(), pc + 4);  // Save return address
            pc = target;
            break;
        }
        
        // --- System Instructions ---
        case FENCE:
        case FENCE_TSO:
        case PAUSE:
            // Memory fence instructions - no-op in our simple emulator
            pc += 4;
            break;
            
        case ECALL:
        case EBREAK:
            // System calls - treat as no-op for now
            pc += 4;
            break;
            
        case RET:
            // Should not reach here as RET is handled in execute()
            break;
            
        default:
            throw std::runtime_error("Unimplemented instruction");
    }
}

uint32_t vmcore::read_reg(uint8_t reg) const {
    if (reg == 0) return 0;  // x0 is always 0
    return registers[reg];
}

void vmcore::write_reg(uint8_t reg, uint32_t value) {
    if (reg == 0) return;  // Writing to x0 is ignored
    registers[reg] = value;
}

uint32_t vmcore::fetch() {
    uint32_t inst = memory.read32(pc);
    return inst;
}

void vmcore::advance_pc() {
    pc += 4;
}

void vmcore::branch(int32_t offset) {
    pc += offset;
}

void vmcore::jump(uint32_t target) {
    pc = target;
}
