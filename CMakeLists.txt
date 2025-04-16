cmake_minimum_required(VERSION 3.30)
project(CHAsm)

set(CMAKE_CXX_STANDARD 11)

# ---- Native target: CHAsm emulator ----
add_executable(CHAsm
        main.cpp
        src/vm/vmcore.cpp
        src/vm/vmcore.h
        src/ISA_CONFIG.cpp
        src/ISA_CONFIG.h
)

# ---- Native target: RV32I Disassembler ----
add_executable(disassembler
        emulator/dis_rv32i.cpp
        emulator/dis_rv32i.h
        emulator/disassembler.cpp
        src/ISA_CONFIG.cpp
        src/ISA_CONFIG.h
)

add_custom_target(all_native
        DEPENDS CHAsm disassembler
)

# ---- Manual RISC-V cross-compile target ----
set(RISCV_COMPILER /opt/riscv-multilib/bin/riscv64-unknown-elf-gcc)
set(RISCV_ARCH "rv32i" CACHE STRING "RISC-V arch (rv32i or rv32e)")

set(RISCV_SOURCE "/home/tgsp/Documents/Syracuse/2025_Spring/CompArch/TermPaper/CHAsm/riscv-experimentaiton/riscv-test.c")
set(RISCV_OUTPUT "${CMAKE_BINARY_DIR}/riscv_test")

# Custom command: compile rv32i test
add_custom_command(
        OUTPUT "${RISCV_OUTPUT}"
        COMMAND "${RISCV_COMPILER}" "${RISCV_SOURCE}"
        -o "${RISCV_OUTPUT}"
        -march="${RISCV_ARCH}"
        -mabi=ilp32
#        -static
        DEPENDS "${RISCV_SOURCE}"
        COMMENT "Cross-compiling riscv-test.c for RISC-V ${RISCV_ARCH}"
)

# Custom target depends on the above output file
add_custom_target(riscv_test_build DEPENDS "${RISCV_OUTPUT}")

# Optional: run RISC-V binary via QEMU
add_custom_target(run_riscv_test
        COMMAND qemu-riscv32 "${RISCV_OUTPUT}"
        DEPENDS riscv_test_build
        COMMENT "Running riscv_test via QEMU"
)

