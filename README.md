# RV32I Virtual Obfuscator and Emulator

A RISC-V RV32I instruction emulator and virtualization toolchain for obfuscation purposes.

## Overview

This project provides tools to:
- Disassemble RISC-V RV32I binaries
- Execute RISC-V functions in a virtual machine
- Emulate complete RV32I instruction set

## Components

### 1. `disassembler`
Disassembles RV32I binary files into human-readable assembly.

```bash
./build/disassembler <binary_file> [base_address]
```

Example:
```bash
./build/disassembler only_fn_riscv_test.rv32i
```

### 2. `emu_rv32i`
Executes a RISC-V function from a binary file and prints the return value.

```bash
./build/emu_rv32i <function.rv32i>
```

The emulator:
- Decodes instructions using `dis_rv32i`
- Executes them in a virtual machine (`vmcore`)
- Passes arguments in registers a0 (x10) and a1 (x11)
- Returns the value in register a0 when a `ret` instruction is encountered

Example:
```bash
./build/emu_rv32i only_fn_riscv_test.rv32i
# Output: 11
```

### 3. `emulator.so`
Shared library containing the virtual machine core and instruction decoder.

## Building

### Prerequisites
- CMake 3.20 or higher
- C++17 compatible compiler (GCC or Clang)
- RISC-V cross-compilation toolchain (optional, for building test functions)

### Build Steps

```bash
mkdir -p build
cd build
cmake ..
make
```

This will build:
- `disassembler` - RV32I disassembler
- `emu_rv32i` - RV32I emulator
- `emulator.so` - Shared library
- `execrv32i` - Alternative emulator frontend

## Testing

### Test 1: Simple Return Value
```bash
./build/emu_rv32i only_fn_riscv_test.rv32i
# Expected output: 11
```

This tests a simple function that returns the constant value 11.

### Test 2: Complex Function (doOperation)
```bash
./build/emu_rv32i build/output/doOperation.rv32i
# Expected output: 46
```

This tests the `doOperation(5, 3)` function which performs:
```c
int doOperation(int a, int b) {
    int c = 7*a;        // c = 35
    int *d = &b;
    int e = *d + 11;    // e = 14
    if (c > e) {        // 35 > 14, true
        c = c - 3;      // c = 32
    } else {
        c = c + 3;
    }
    return c + e;       // 32 + 14 = 46
}
```

## Supported Instructions

The emulator supports the complete RV32I instruction set:

### Arithmetic
- `ADD`, `SUB`, `ADDI`
- `SLT`, `SLTU`, `SLTI`, `SLTIU`

### Logical
- `AND`, `OR`, `XOR`, `ANDI`, `ORI`, `XORI`

### Shifts
- `SLL`, `SRL`, `SRA`, `SLLI`, `SRLI`, `SRAI`

### Memory
- Loads: `LW`, `LH`, `LB`, `LHU`, `LBU`
- Stores: `SW`, `SH`, `SB`

### Branches
- `BEQ`, `BNE`, `BLT`, `BGE`, `BLTU`, `BGEU`

### Jumps
- `JAL`, `JALR`, `RET`

### Upper Immediate
- `LUI`, `AUIPC`

### System
- `ECALL`, `EBREAK` (treated as no-ops)
- `FENCE`, `FENCE.TSO`, `PAUSE` (no-ops in this emulator)

## Architecture

### vmcore
The virtual machine core (`src/vmcore.cpp`) provides:
- 32 general-purpose registers (x0-x31)
- Virtual memory system with code, data, heap, and stack sections
- Full RV32I instruction execution
- Program counter and control flow management

### dis_rv32i
The instruction decoder (`src/dis_rv32i.cpp`) provides:
- Instruction type identification (I, R, S, B, U, J, Fence, System)
- Field extraction (opcodes, registers, immediates)
- Mnemonic identification
- String representation for disassembly

### VirtualMemory
The memory subsystem (`src/VirtMem.cpp`) provides:
- Auto-growing memory allocation
- 8-bit, 16-bit, and 32-bit access methods
- Stack at 8MB (grows downward)
- Heap at 16MB
- Code section at 64KB

## Creating Test Binaries

To create your own RV32I test binaries:

1. Write a C function:
```c
int my_function(int a, int b) {
    return a + b;
}
```

2. Compile to RISC-V:
```bash
clang -target riscv32-unknown-elf -march=rv32i -mabi=ilp32 -O0 -c my_function.c -o my_function.o
clang -target riscv32-unknown-elf -march=rv32i -mabi=ilp32 -nostdlib -nostartfiles -static my_function.o -o my_function.elf
```

3. Extract the .text section:
```bash
objcopy -O binary --only-section=.text my_function.elf my_function.rv32i
```

4. Run with the emulator:
```bash
./build/emu_rv32i my_function.rv32i
```

## Project Structure

```
.
├── CMakeLists.txt          # Build configuration
├── emu_rv32i.cpp           # Emulator main program
├── main.cpp                # Alternative emulator frontend
├── only_fn_riscv_test.rv32i # Test binary (returns 11)
├── src/
│   ├── dis_rv32i.cpp       # Instruction decoder
│   ├── dis_rv32i.h         # Decoder header
│   ├── disassembler.cpp    # Disassembler tool
│   ├── vmcore.cpp          # VM core implementation
│   ├── vmcore.h            # VM core header
│   ├── VirtMem.cpp         # Virtual memory
│   ├── VirtMem.h           # Virtual memory header
│   ├── cpu_rv32i.cpp       # CPU implementation (unused)
│   ├── cpu_rv32i.h         # CPU header (unused)
│   ├── ISA_CONFIG.cpp      # ISA configuration
│   └── ISA_CONFIG.h        # ISA configuration header
├── target/                 # Test C source files
│   ├── doOperation.c
│   ├── riscv-test.c
│   └── only_fn_riscv_test.c
└── build/                  # Build output (generated)
    ├── disassembler
    ├── emu_rv32i
    ├── emulator.so
    └── output/             # Compiled RISC-V binaries
        ├── doOperation.rv32i
        ├── doOperation.elf
        └── ...
```

## License

[License information to be added]

## Authors

- TownsendSP

## Contributing

[Contribution guidelines to be added]
