// execrv32i - RV32I Disassembler and Emulator
// Usage:
//   execrv32i dis <function.rv32i> [base_address]
//   execrv32i emu <function.rv32i> [arg1] [arg2] ...

#include "argparse.hpp"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "src/obf/obfuscate.h"
#include "src/obf/restore.h"
#include "src/rv32i/cpu_rv32i.h"
#include "src/rv32i/dis_rv32i.h"
#include "src/rv32i/regs_rv32i.h"


// Reads a binary file and returns its contents as a vector of bytes

std::vector<uint8_t> read_binary_file(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filepath);
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(size);
  if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
    throw std::runtime_error("Failed to read file: " + filepath);
  }

  return buffer;
}


// Disassembles a binary buffer into a vector of Instruction objects
// Assumes little-endian byte order

std::vector<std::unique_ptr<Instruction>>
disassemble(const std::vector<uint8_t> &binary, uint32_t baseAddress = 0) {
  std::vector<std::unique_ptr<Instruction>> instructions;

  // 4 byte alignment
  if (binary.size() % 4 != 0) {
    std::cerr << "Warning: Binary size is not a multiple of 4 bytes"
              << std::endl;
  }

  for (size_t i = 0; i + 3 < binary.size(); i += 4) {
    // Read 32-bit instruction in little-endian format
    uint32_t raw = static_cast<uint32_t>(binary[i]) |
                   (static_cast<uint32_t>(binary[i + 1]) << 8) |
                   (static_cast<uint32_t>(binary[i + 2]) << 16) |
                   (static_cast<uint32_t>(binary[i + 3]) << 24);

    try {
      instructions.push_back(Instruction::create(raw));
    } catch (const std::invalid_argument &e) {
      std::cerr << "Warning at offset 0x" << std::hex << (baseAddress + i)
                << ": " << e.what() << " (raw: 0x" << std::setfill('0')
                << std::setw(8) << raw << ")" << std::dec << std::endl;
    }
  }

  return instructions;
}

void print_disassembly(
    const std::vector<std::unique_ptr<Instruction>> &instructions,
    uint32_t baseAddress = 0, bool only_asm = false) {
  uint32_t addr = baseAddress;
  for (const auto &instr : instructions) {
    if (!only_asm) {
      std::cout << std::hex << std::setfill('0') << std::setw(8) << addr
                << ":  " << std::setw(8) << instr->getRaw() << "  " << std::dec;
    }
    std::cout << instr->toString();

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

void run_disassemble(const std::string &filepath, uint32_t baseAddress,
                     bool is_obfuscated, bool only_asm) {
  std::vector<uint8_t> data = read_binary_file(filepath);
  if (is_obfuscated) {
    restore(data);
    std::cout << "Deobfuscated input file before processing.\n";
  }

  std::cout << "Loaded " << data.size() << " bytes" << std::endl;
  std::cout << std::endl;

  std::vector<std::unique_ptr<Instruction>> instructions =
      disassemble(data, baseAddress);
  std::cout << "Disassembled " << instructions.size()
            << " instructions:" << std::endl;

  std::cout << std::string(60, '-') << std::endl;
  print_disassembly(instructions, baseAddress, only_asm);
}

void run_emulate(const std::string &filepath,
                 const std::vector<std::string> &args, bool is_obfuscated) {
  std::vector<uint8_t> binary = read_binary_file(filepath);
  if (is_obfuscated) {
    restore(binary);
    std::cout << "Deobfuscated input file before processing.\n";
  }

  mem_rv32i::init();

  // disassemble
  std::vector<std::unique_ptr<Instruction>> instructions;
  if (binary.size() % 4 != 0) {
    throw std::runtime_error("Binary size is not a multiple of 4");
  }

  for (size_t i = 0; i < binary.size(); i += 4) {
    uint32_t raw = 0;
    // Little-endian load
    raw |= binary[i];
    raw |= (uint32_t)binary[i + 1] << 8;
    raw |= (uint32_t)binary[i + 2] << 16;
    raw |= (uint32_t)binary[i + 3] << 24;

    instructions.push_back(Instruction::create(raw));
  }

  cpu_rv32i vm;
  vm.load_program(binary);

  // args are passed in a0-a7 (x10-x17)
  int arg_reg_start = 10;
  int max_args = 8;

  for (size_t i = 0; i < args.size() && i < (size_t)max_args; ++i) {
    try {
      uint32_t val = std::stoul(args[i], nullptr, 0);
      vm.write_reg(arg_reg_start + i, val);
    } catch (const std::exception &e) {
      std::cerr << "Warning: Failed to parse argument '" << args[i]
                << "': " << e.what() << "\n";
    }
  }

  vm.execute(instructions);
  uint32_t result = vm.read_reg(10); // a0

  std::cout << result << std::endl;
}

void obfuscate_file(const std::string &input_path,
                    const std::string &output_path) {
  std::vector<uint8_t> data = read_binary_file(input_path);
  std::vector<uint8_t> obfuscated = obfuscate(data);

  std::ofstream out(output_path, std::ios::binary);
  if (!out)
    throw std::runtime_error("Failed to open output file: " + output_path);
  out.write(reinterpret_cast<const char *>(obfuscated.data()),
            obfuscated.size());
  std::cout << "Obfuscated " << data.size() << " bytes to " << output_path
            << std::endl;
}

void deobfuscate_file(const std::string &input_path,
                      const std::string &output_path) {
  std::vector<uint8_t> data = read_binary_file(input_path);
  restore(data);

  std::ofstream out(output_path, std::ios::binary);
  if (!out)
    throw std::runtime_error("Failed to open output file: " + output_path);
  out.write(reinterpret_cast<const char *>(data.data()), data.size());
  std::cout << "Deobfuscated " << data.size() << " bytes to " << output_path
            << std::endl;
}

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("execrv32i");

  argparse::ArgumentParser dis_command("dis");
  dis_command.add_description("Disassemble a RV32I binary file");
  dis_command.add_argument("binary").help("Path to the RV32I binary file");
  dis_command.add_argument("base_address")
      .help("Base address for disassembly (hex)")
      .default_value(std::string("0"));
  dis_command.add_argument("--obfuscated")
      .help("Deobfuscate the input file before processing")
      .default_value(false)
      .implicit_value(true);
  dis_command.add_argument("--onlyasm")
      .help("Only Output the assembly, omitting the address and hex columns")
      .default_value(false)
      .implicit_value(true);

  argparse::ArgumentParser emu_command("emu");
  emu_command.add_description(
      "Emulate a RV32I function with optional arguments");
  emu_command.add_argument("binary").help("Path to the RV32I binary file");
  emu_command.add_argument("args")
      .help("Arguments to pass to the function")
      .remaining();
  emu_command.add_argument("--obfuscated")
      .help("Deobfuscate the input file before processing")
      .default_value(false)
      .implicit_value(true);

  argparse::ArgumentParser obf_command("obf");
  obf_command.add_description("Obfuscate a rv32i file");
  obf_command.add_argument("input").help("Input rv32i file");
  obf_command.add_argument("output").help("Output obfuscated rv32i file");

  argparse::ArgumentParser deobf_command("deobf");
  deobf_command.add_description("Deobfuscate a rv32i file");
  deobf_command.add_argument("input").help("Input obfuscated .obf.rv32i file");
  deobf_command.add_argument("output").help("Output deobfuscated .rv32i file");

  program.add_subparser(dis_command);
  program.add_subparser(emu_command);
  program.add_subparser(obf_command);
  program.add_subparser(deobf_command);

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  try {
    if (program.is_subcommand_used(dis_command)) {
      std::string binary = dis_command.get<std::string>("binary");
      std::string base_addr_str = dis_command.get<std::string>("base_address");
      bool obfuscated = dis_command.get<bool>("--obfuscated");

      bool only_asm = dis_command.get<bool>("--onlyasm");

      uint32_t base_address = 0;
      try {
        base_address = std::stoul(base_addr_str, nullptr, 16);
      } catch (...) {
        std::cerr << "Invalid base address: " << base_addr_str << std::endl;
        return 1;
      }

      run_disassemble(binary, base_address, obfuscated, only_asm);
    } else if (program.is_subcommand_used(emu_command)) {
      std::string binary = emu_command.get<std::string>("binary");
      bool obfuscated = emu_command.get<bool>("--obfuscated");
      std::vector<std::string> args;
      try {
        args = emu_command.get<std::vector<std::string>>("args");
      } catch (const std::logic_error &e) {
      }

      run_emulate(binary, args, obfuscated);
    } else if (program.is_subcommand_used(obf_command)) {
      std::string input = obf_command.get<std::string>("input");
      std::string output = obf_command.get<std::string>("output");
      obfuscate_file(input, output);
    } else if (program.is_subcommand_used(deobf_command)) {
      std::string input = deobf_command.get<std::string>("input");
      std::string output = deobf_command.get<std::string>("output");
      deobfuscate_file(input, output);
    } else {
      std::cerr << program;
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
