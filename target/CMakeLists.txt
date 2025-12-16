cmake_minimum_required(VERSION 3.20)
project(demo_test)
add_compile_options(-g)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# ============================================================================
# TOOLCHAIN CONFIGURATION (Copied from root)
# ============================================================================
set(RISCV_C_COMPILER "clang")
set(RISCV_OBJCOPY "llvm-objcopy")
set(RISCV_OBJDUMP "llvm-objdump")

set(RISCV_ARCH "rv32i")
set(RISCV_ABI "ilp32")

set(RISCV_TARGET_FLAGS -target riscv32-unknown-elf -march=${RISCV_ARCH} -mabi=${RISCV_ABI})
set(RISCV_LINK_FLAGS -nostdlib -nostartfiles -static)
set(RISCV_C_FLAGS ${RISCV_TARGET_FLAGS} -O0 -Wall -g)

# ============================================================================
# DIRECTORIES
# ============================================================================
set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(SRC_DIR "${ROOT_DIR}/src")

include_directories(${SRC_DIR}/rv32i)
include_directories(${SRC_DIR})

# ============================================================================
# EMULATOR LIBRARY
# ============================================================================
add_library(emulator_static STATIC
    ${SRC_DIR}/rv32i/cpu_rv32i.cpp
    ${SRC_DIR}/rv32i/mem_rv32i.cpp
    ${SRC_DIR}/rv32i/dis_rv32i.cpp
    ${SRC_DIR}/rv32i/emulator_api.cpp
    ${SRC_DIR}/obf/restore.cpp
)

# ============================================================================
# OBFUSCATOR TOOL
# ============================================================================
add_executable(obfuscator_tool
    ${ROOT_DIR}/main.cpp
    ${SRC_DIR}/obf/obfuscate.cpp
    ${SRC_DIR}/obf/restore.cpp
)
target_link_libraries(obfuscator_tool emulator_static)

# ============================================================================
# OBFUSCATION PIPELINE
# ============================================================================

# 1. Compile, Link, and Extract addition.c
add_custom_command(
    OUTPUT doOperation.rv32i doOperation.elf
    COMMAND ${RISCV_C_COMPILER} ${RISCV_C_FLAGS} -c ${CMAKE_CURRENT_SOURCE_DIR}/doOperation.c -o doOperation.o
    COMMAND ${RISCV_C_COMPILER} ${RISCV_C_FLAGS} ${RISCV_LINK_FLAGS} doOperation.o -o doOperation.elf
    COMMAND ${RISCV_OBJCOPY} -O binary --only-section=.text doOperation.elf doOperation.rv32i
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/doOperation.c
    COMMENT "Building doOperation.rv32i (Compile -> Link -> Extract)"
)

# 2. Obfuscate
add_custom_command(
    OUTPUT doOperation.obf.rv32i
    COMMAND ./obfuscator_tool obf doOperation.rv32i doOperation.obf.rv32i
    DEPENDS doOperation.rv32i obfuscator_tool
    COMMENT "Obfuscating doOperation.rv32i -> doOperation.obf.rv32i"
)

# 4. Generate Trampoline
add_custom_command(
    OUTPUT trampoline_doOperation.c
    COMMAND python3 ${SRC_DIR}/gen_trampoline.py
        --header ${CMAKE_CURRENT_SOURCE_DIR}/doOperation.h
        --function doOperation
        --bytecode doOperation.obf.rv32i
        --output trampoline_doOperation.c
    DEPENDS doOperation.obf.rv32i ${SRC_DIR}/gen_trampoline.py
    COMMENT "Generating trampoline_doOperation.c"
)

# ============================================================================
# FINAL EXECUTABLE
# ============================================================================
add_executable(demo_test
    demo_test.c
    trampoline_doOperation.c
)

target_link_libraries(demo_test emulator_static)
