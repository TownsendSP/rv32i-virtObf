// execrv32i - RV32I Virtualized Function Executor
// Usage: execrv32i <function.rv32i> --arguments <args.txt>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

// TODO: Include emulator interface when vmcore.h is implemented
// #include "src/vmcore.h"
// #include "src/dis_rv32i.h"

void print_usage(const char* progname) {
    std::cerr << "Usage: " << progname << " <function.rv32i> --arguments <args.txt>\n";
    std::cerr << "\n";
    std::cerr << "Arguments:\n";
    std::cerr << "  function.rv32i  - Path to virtualized RV32I function binary\n";
    std::cerr << "  --arguments     - Path to arguments file (format: space-separated integers)\n";
    std::cerr << "\n";
    std::cerr << "Example:\n";
    std::cerr << "  " << progname << " output/doOperation.rv32i --arguments args.txt\n";
    std::cerr << "\n";
    std::cerr << "Arguments file format (args.txt):\n";
    std::cerr << "  5 3\n";
    std::cerr << "  (This passes two integer arguments: a=5, b=3)\n";
}

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

std::vector<int32_t> parse_arguments(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open arguments file: " + filepath);
    }
    
    std::vector<int32_t> args;
    int32_t value;
    while (file >> value) {
        args.push_back(value);
    }
    
    return args;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    std::string rv32i_path;
    std::string args_path;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--arguments") == 0 || std::strcmp(argv[i], "-a") == 0) {
            if (i + 1 < argc) {
                args_path = argv[++i];
            } else {
                std::cerr << "Error: --arguments requires a file path\n";
                return 1;
            }
        } else if (rv32i_path.empty()) {
            rv32i_path = argv[i];
        } else {
            std::cerr << "Error: Unknown argument: " << argv[i] << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (rv32i_path.empty()) {
        std::cerr << "Error: No RV32I function file specified\n";
        print_usage(argv[0]);
        return 1;
    }
    
    try {
        // Load the RV32I binary
        std::cout << "Loading RV32I function: " << rv32i_path << "\n";
        std::vector<uint8_t> function_binary = read_binary_file(rv32i_path);
        std::cout << "  Loaded " << function_binary.size() << " bytes\n";
        
        // Load arguments if specified
        std::vector<int32_t> args;
        if (!args_path.empty()) {
            std::cout << "Loading arguments: " << args_path << "\n";
            args = parse_arguments(args_path);
            std::cout << "  Loaded " << args.size() << " arguments: ";
            for (size_t i = 0; i < args.size(); i++) {
                std::cout << args[i];
                if (i < args.size() - 1) std::cout << ", ";
            }
            std::cout << "\n";
        }
        
        // TODO: Disassemble the binary to Instruction objects
        // std::vector<std::unique_ptr<Instruction>> instructions = disassemble(function_binary);
        
        // TODO: Initialize the emulator
        // vmcore emulator;
        
        // TODO: Load instructions into emulator
        // emulator.load_program(instructions);
        
        // TODO: Set up arguments (convention: a0=arg0, a1=arg1, etc.)
        // for (size_t i = 0; i < args.size() && i < 8; i++) {
        //     emulator.set_register(10 + i, args[i]);  // a0-a7 are x10-x17
        // }
        
        // TODO: Execute the virtualized function
        // emulator.execute();
        
        // TODO: Get the return value (convention: a0 holds return value)
        // int32_t result = emulator.get_register(10);  // a0 is x10
        
        std::cout << "\n[NOT YET IMPLEMENTED]\n";
        std::cout << "TODO: Emulator execution will happen here\n";
        std::cout << "TODO: Disassemble binary -> Instruction objects\n";
        std::cout << "TODO: Initialize vmcore emulator\n";
        std::cout << "TODO: Load program and set arguments\n";
        std::cout << "TODO: Execute and retrieve result\n";
        
        // Placeholder return
        // std::cout << "\nResult: " << result << "\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}