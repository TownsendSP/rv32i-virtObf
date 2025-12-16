#include "emulator_api.h"
#include "cpu_rv32i.h"
#include "dis_rv32i.h"
#include "../obf/restore.h"
#include <cstdarg>
#include <vector>
#include <cstring>
#include <iostream>

extern "C" {

uint32_t rv32i_call(const uint8_t* bytecode, size_t size, ...) {
    cpu_rv32i cpu;

    // load
    std::vector<uint8_t> code(bytecode, bytecode + size);
    deobfuscate(code);
    cpu.load_program(code);

    // Decode instructions from the restored code
    std::vector<std::unique_ptr<Instruction>> instructions;
    for (size_t i = 0; i < code.size(); i += 4) {
        uint32_t raw = 0;
        if (i + 4 <= code.size()) {
            // Read from restored code (little-endian)
            raw = code[i] | (code[i+1] << 8) | (code[i+2] << 16) | (code[i+3] << 24);
        } else {
             // Should not happen if aligned
            memcpy(&raw, code.data() + i, code.size() - i);
        }
        instructions.push_back(decodeInstruction(raw));
    }

    // Set arguments
    va_list args;
    va_start(args, size);
    for (int i = 0; i < 8; ++i) {
        uint32_t arg = va_arg(args, uint32_t);
        cpu.write_reg(10 + i, arg); // a0 is x10
    }
    va_end(args);

    // Execute
    try {
        cpu.execute(instructions);
    } catch (const std::exception& e) {
        std::cerr << "Emulator error: " << e.what() << std::endl;
        return 0;
    }

    return cpu.read_reg(10); // return a0
}

uint64_t rv32i_call64(const uint8_t* bytecode, size_t size, ...) {
    cpu_rv32i cpu;

    // Load program into memory
    std::vector<uint8_t> code(bytecode, bytecode + size);
    deobfuscate(code); // Restore obfuscated code
    cpu.load_program(code);

    // Decode instructions from the restored code
    std::vector<std::unique_ptr<Instruction>> instructions;
    for (size_t i = 0; i < code.size(); i += 4) {
        uint32_t raw = 0;
        if (i + 4 <= code.size()) {
            // Read from restored code (little-endian)
            raw = code[i] | (code[i+1] << 8) | (code[i+2] << 16) | (code[i+3] << 24);
        } else {
            memcpy(&raw, code.data() + i, code.size() - i);
        }
        instructions.push_back(decodeInstruction(raw));
    }

    // Set arguments
    va_list args;
    va_start(args, size);
    for (int i = 0; i < 8; ++i) {
        uint32_t arg = va_arg(args, uint32_t);
        cpu.write_reg(10 + i, arg); // a0 is x10
    }
    va_end(args);

    // Execute
    try {
        cpu.execute(instructions);
    } catch (const std::exception& e) {
        std::cerr << "Emulator error: " << e.what() << std::endl;
        return 0;
    }

    uint64_t lo = cpu.read_reg(10);
    uint64_t hi = cpu.read_reg(11);
    return lo | (hi << 32);
}

}
