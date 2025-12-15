//
// Created by tgsp on 4/15/25.
//

#ifndef CPU_RV32I_H
#define CPU_RV32I_H

#include <cstdint>
#include <vector>
#include <stdexcept>

#include "../mem_rv32i.h"
#include "../dis_rv32i.h"

// Main CPU core - executes RV32I instructions
class cpu_rv32i {
public:
    // 32 general-purpose registers (x0-x31)
    // x0 is hardwired to 0
    // x1 (ra) = return address
    // x2 (sp) = stack pointer
    uint32_t registers[32];

    // Program counter
    uint32_t pc;

    // Virtual memory system
    mem_rv32i memory;

    cpu_rv32i();

    // Load a program into memory
    void load_program(const std::vector<uint8_t>& program);

    // Read register (handles x0 special case)
    uint32_t read_reg(uint8_t reg) const;

    // Write register (handles x0 special case)
    void write_reg(uint8_t reg, uint32_t value);

    // Fetch the next instruction
    uint32_t fetch();

    // Advance PC
    void advance_pc();

    // Branch/Jump
    void branch(int32_t offset);

    void jump(uint32_t target);

    // Execute a sequence of instructions
    void execute(const std::vector<std::unique_ptr<Instruction>>& instructions);
};

#endif //CPU_RV32I_H

