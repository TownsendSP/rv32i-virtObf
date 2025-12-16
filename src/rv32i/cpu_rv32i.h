//
// Created by tgsp on 4/15/25.
//

#ifndef CPU_RV32I_H
#define CPU_RV32I_H

#include <cstdint>
#include <vector>
#include <stdexcept>

#include "mem_rv32i.h"
#include "dis_rv32i.h"

// Main CPU core - executes RV32I instructions
class cpu_rv32i {
public:
    uint32_t registers[32];

    uint32_t pc;

    mem_rv32i memory;

    cpu_rv32i();

    void load_program(const std::vector<uint8_t>& program);

    uint32_t read_reg(uint8_t reg) const;

    void write_reg(uint8_t reg, uint32_t value);

    uint32_t fetch();

    void advance_pc();

    void branch(int32_t offset);

    void jump(uint32_t target);

    void execute(const std::vector<std::unique_ptr<Instruction>>& instructions);
};

uint32_t rv32i_call(const uint8_t* bytecode, size_t size,
                    uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3,
                    uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7);

#endif //CPU_RV32I_H

