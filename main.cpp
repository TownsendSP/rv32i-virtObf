// execrv32i - RV32I Disassembler and Emulator
// Usage:
//   execrv32i dis <function.rv32i> [base_address]
//   execrv32i emu <function.rv32i> [arg1] [arg2] ...

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <memory>

#include "src/rv32i/cpu_rv32i.h"
#include "src/rv32i/dis_rv32i.h"
#include "src/obf/obfuscate.h"
#include "src/obf/restore.h"

void print_usage(const char* progname) {
    std::cerr << "RV32I Disassembler and Emulator\n\n";
    std::cerr << "Usage:\n";
    std::cerr << "  " << progname << " dis <binary.rv32i> [base_address]\n";
    std::cerr << "  " << progname << " emu <binary.rv32i> [arg1] [arg2] ...\n";
    std::cerr << "\n";
    std::cerr << "Commands:\n";
    std::cerr << "  dis    - Disassemble a RV32I binary file\n";
    std::cerr << "  emu    - Emulate a RV32I function with optional arguments\n";
    std::cerr << "  obf    - Obfuscate a binary file\n";
    std::cerr << "  deobf  - Deobfuscate a binary file\n";
    std::cerr << "\n";
    std::cerr << "Examples:\n";
    std::cerr << "  " << progname << " dis output/doOperation.rv32i\n";
    std::cerr << "  " << progname << " dis output/doOperation.rv32i 0x10000\n";
    std::cerr << "  " << progname << " emu testing_utils/only_fn_riscv_test.rv32i 5 6\n";
    std::cerr << "  " << progname << " obf input.rv32i output.obf\n";
    std::cerr << "  " << progname << " deobf input.obf output.rv32i\n";
    std::cerr << "\n";
}

/**
 * Reads a binary file and returns its contents as a vector of bytes
 */
std::vector<uint8_t> read_binary_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file: " + filepath);
    }
    
    return buffer;
}

/**
 * Disassembles a binary buffer into a vector of Instruction objects
 * Assumes little-endian byte order
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
        }
    }

    return instructions;
}

/**
 * Prints disassembly with addresses
 */
void print_disassembly(const std::vector<std::unique_ptr<Instruction>>& instructions, uint32_t baseAddress = 0) {
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

bool disassemble(int argc, char **argv, std::string filepath, int &value1) {
    uint32_t baseAddress = 0;

    // Parse optional base address
    if (argc >= 4) {
        try {
            baseAddress = std::stoul(argv[3], nullptr, 16);
        } catch (const std::exception& e) {
            std::cerr << "Invalid base address: " << argv[3] << std::endl;
            value1 = 1;
            return true;
        }
    }
    std::vector<uint8_t> binary = read_binary_file(filepath);
    std::cout << "Loaded " << binary.size() << " bytes from " << filepath << std::endl;
    std::cout << std::endl;

    std::vector<std::unique_ptr<Instruction>> instructions = disassemble(binary, baseAddress);
    std::cout << "Disassembled " << instructions.size() << " instructions:" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    print_disassembly(instructions, baseAddress);
    return false;
}

void emulate(int argc, char **argv, std::string filepath) {
    mem_rv32i::init();
    std::vector<uint8_t> binary = read_binary_file(filepath);

    // disassemble
    std::vector<std::unique_ptr<Instruction>> instructions;
    if (binary.size() % 4 != 0) {
        throw std::runtime_error("Binary size is not a multiple of 4");
    }

    for (size_t i = 0; i < binary.size(); i += 4) {
        uint32_t raw = 0;
        // Little-endian load
        raw |= binary[i];
        raw |= (uint32_t)binary[i+1] << 8;
        raw |= (uint32_t)binary[i+2] << 16;
        raw |= (uint32_t)binary[i+3] << 24;

        instructions.push_back(Instruction::create(raw));
    }

    cpu_rv32i vm;
    vm.load_program(binary);

    // args are passed in a0-a7 (x10-x17)
    int arg_reg_start = 10;
    int max_args = 8;

    for (int i = 3; i < argc && (i - 3) < max_args; ++i) {
        try {
            // Support both decimal and hex (via 0 base)
            uint32_t val = std::stoul(argv[i], nullptr, 0);
            vm.write_reg(arg_reg_start + (i - 3), val);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to parse argument '" << argv[i] << "': " << e.what() << "\n";
        }
    }

    vm.execute(instructions);
    uint32_t result = vm.read_reg(10); // a0

    std::cout << result << std::endl;
}

void obfuscate_file(const std::string& input_path, const std::string& output_path) {
    std::vector<uint8_t> data = read_binary_file(input_path);
    std::vector<uint8_t> obfuscated = obfuscate(data);
    
    std::ofstream out(output_path, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to open output file: " + output_path);
    out.write(reinterpret_cast<const char*>(obfuscated.data()), obfuscated.size());
    std::cout << "Obfuscated " << data.size() << " bytes to " << output_path << std::endl;
}

void deobfuscate_file(const std::string& input_path, const std::string& output_path) {
    std::vector<uint8_t> data = read_binary_file(input_path);
    restore(data);
    
    std::ofstream out(output_path, std::ios::binary);
    if (!out) throw std::runtime_error("Failed to open output file: " + output_path);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    std::cout << "Deobfuscated " << data.size() << " bytes to " << output_path << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    std::string filepath = argv[2];

    try {
        if (command == "dis") {
            int retcode;
            if (disassemble(argc, argv, filepath, retcode)) return retcode;
        } else if (command == "emu") {
            emulate(argc, argv, filepath);
        } else if (command == "obf") {
            if (argc < 4) {
                std::cerr << "Error: Missing output file for obf\n";
                return 1;
            }
            obfuscate_file(filepath, argv[3]);
        } else if (command == "deobf") {
            if (argc < 4) {
                std::cerr << "Error: Missing output file for deobf\n";
                return 1;
            }
            deobfuscate_file(filepath, argv[3]);

        } else {
            std::cerr << "Error: Unknown command '" << command << "'\n";
            print_usage(argv[0]);
            return 1;
        }

        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
