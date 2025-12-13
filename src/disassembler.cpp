//
// Created by tgsp on 4/16/25.
//

#include "dis_rv32i.h"

/*
 * This file contains all the tests for each instruction in the disassembler
 */

#include "dis_rv32i.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstring>

/**
 * Reads a binary file and returns its contents as a vector of bytes
 */
std::vector<uint8_t> readBinaryFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + filename);
    }

    return buffer;
}

/**
 * Disassembles a binary buffer into a vector of Instruction objects
 * Assumes little-endian byte order (standard for RISC-V)
 */
std::vector<std::unique_ptr<Instruction>> disassemble(const std::vector<uint8_t>& binary, uint32_t baseAddress = 0) {
    std::vector<std::unique_ptr<Instruction>> instructions;

    // RISC-V instructions are 4 bytes aligned
    if (binary.size() % 4 != 0) {
        std::cerr << "Warning: Binary size is not a multiple of 4 bytes" << std::endl;
    }

    for (size_t i = 0; i + 3 < binary.size(); i += 4) {
        // Read 32-bit instruction in little-endian format
        uint32_t raw = static_cast<uint32_t>(binary[i])
                     | (static_cast<uint32_t>(binary[i + 1]) << 8)
                     | (static_cast<uint32_t>(binary[i + 2]) << 16)
                     | (static_cast<uint32_t>(binary[i + 3]) << 24);

        try {
            instructions.push_back(Instruction::create(raw));
        } catch (const std::invalid_argument& e) {
            std::cerr << "Warning at offset 0x" << std::hex << (baseAddress + i)
                      << ": " << e.what()
                      << " (raw: 0x" << std::setfill('0') << std::setw(8) << raw << ")"
                      << std::dec << std::endl;
            // Continue parsing - skip this instruction
        }
    }

    return instructions;
}

/**
 * Prints disassembly with addresses
 */
void printDisassembly(const std::vector<std::unique_ptr<Instruction>>& instructions, uint32_t baseAddress = 0) {
    uint32_t addr = baseAddress;
    for (const auto& instr : instructions) {
        std::cout << std::hex << std::setfill('0') << std::setw(8) << addr << ":  "
                  << std::setw(8) << instr->getRaw() << "  "
                  << std::dec << instr->toString();

        // Show control flow info
        if (instr->isBranch() || instr->isJump()) {
            int32_t offset = instr->getImmediate();
            uint32_t target = addr + offset;
            std::cout << "  # target: 0x" << std::hex << target << std::dec;
        }

        std::cout << std::endl;
        addr += 4;
    }
}

void printUsage(const char* progName) {
    std::cerr << "Usage: " << progName << " <binary_file> [base_address]" << std::endl;
    std::cerr << "  binary_file   - Path to RV32I binary file" << std::endl;
    std::cerr << "  base_address  - Optional base address in hex (default: 0)" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string filename = argv[1];
    uint32_t baseAddress = 0;

    // Parse optional base address
    if (argc >= 3) {
        try {
            baseAddress = std::stoul(argv[2], nullptr, 16);
        } catch (const std::exception& e) {
            std::cerr << "Invalid base address: " << argv[2] << std::endl;
            return 1;
        }
    }

    try {
        // Read the binary file
        std::vector<uint8_t> binary = readBinaryFile(filename);
        std::cout << "Loaded " << binary.size() << " bytes from " << filename << std::endl;
        std::cout << std::endl;

        // Disassemble
        std::vector<std::unique_ptr<Instruction> > instructions = disassemble(binary, baseAddress);
        std::cout << "Disassembled " << instructions.size() << " instructions:" << std::endl;
        std::cout << std::string(60, '-') << std::endl;

        // Print disassembly
        printDisassembly(instructions, baseAddress);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}