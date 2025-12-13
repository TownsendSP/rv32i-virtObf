// emu_rv32i.cpp - RISC-V Instruction Emulator
// Usage: emu_rv32i <function.rv32i>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

#include "src/vmcore.h"
#include "src/dis_rv32i.h"

/**
 * Reads a binary file and returns its contents as a vector of bytes
 */
std::vector<uint8_t> read_binary_file(const std::string& filename) {
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
std::vector<std::unique_ptr<Instruction>> disassemble(const std::vector<uint8_t>& binary) {
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
            std::cerr << "Warning at offset 0x" << std::hex << i
                      << ": " << e.what()
                      << " (raw: 0x" << raw << ")"
                      << std::dec << std::endl;
            // Continue parsing - skip this instruction
        }
    }
    
    return instructions;
}

void print_usage(const char* progname) {
    std::cerr << "Usage: " << progname << " <function.rv32i>\n";
    std::cerr << "\n";
    std::cerr << "Arguments:\n";
    std::cerr << "  function.rv32i  - Path to RISC-V function binary\n";
    std::cerr << "\n";
    std::cerr << "Example:\n";
    std::cerr << "  " << progname << " only_fn_riscv_test.rv32i\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    std::string rv32i_path = argv[1];
    
    try {
        // Load the RV32I binary
        std::vector<uint8_t> function_binary = read_binary_file(rv32i_path);
        
        // Disassemble the binary to Instruction objects
        std::vector<std::unique_ptr<Instruction>> instructions = disassemble(function_binary);
        
        // Initialize the emulator
        vmcore emulator;
        
        // Execute the virtualized function with arguments 5 and 3
        // (These are the standard test arguments for doOperation)
        int32_t result = emulator.execute(instructions, 5, 3);
        
        // Print the result
        std::cout << result << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
