cmake_minimum_required(VERSION 3.20)
project(rv32i_VirtualizingObfuscator)

# Dependencies
include_directories(${CMAKE_SOURCE_DIR}/include)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(RISCV_C_COMPILER "clang")
set(RISCV_CXX_COMPILER "clang++")
set(RISCV_OBJCOPY "llvm-objcopy")
set(RISCV_OBJDUMP "llvm-objdump")

# RISC-V Architecture Configuration
set(RISCV_ARCH "rv32i" CACHE STRING "RISC-V architecture (rv32i, rv32im, etc.)")
set(RISCV_ABI "ilp32" CACHE STRING "RISC-V ABI (ilp32 for RV32I)")

# --- Native flags
set(NATIVE_C_FLAGS -Wall -Wextra)
set(NATIVE_CXX_FLAGS -Wall -Wextra)

# --- rv32i flags
set(RISCV_TARGET_FLAGS -target riscv32-unknown-elf -march=${RISCV_ARCH} -mabi=${RISCV_ABI})
set(RISCV_LINK_FLAGS -nostdlib -nostartfiles -static -fuse-ld=lld)
set(RISCV_C_FLAGS ${RISCV_TARGET_FLAGS} -O0 -Wall)
set(RISCV_CXX_FLAGS ${RISCV_TARGET_FLAGS} -O0 -Wall)

# DIRECTORIES
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")

file(MAKE_DIRECTORY ${OUTPUT_DIR})

# Common Code
set(COMMON_SOURCES
        ${SRC_DIR}/rv32i/dis_rv32i.cpp
        ${SRC_DIR}/rv32i/dis_rv32i.h
)

# Emulator (Shared)
add_library(emulator SHARED
        ${SRC_DIR}/rv32i/cpu_rv32i.cpp
        ${SRC_DIR}/rv32i/cpu_rv32i.h
        ${SRC_DIR}/rv32i/mem_rv32i.cpp
        ${SRC_DIR}/rv32i/mem_rv32i.h
        ${SRC_DIR}/obf/restore.cpp
        ${SRC_DIR}/obf/restore.h
        ${COMMON_SOURCES}
)

# Emulator (Static)
add_library(emulator_static STATIC
        ${SRC_DIR}/rv32i/cpu_rv32i.cpp
        ${SRC_DIR}/rv32i/cpu_rv32i.h
        ${SRC_DIR}/rv32i/mem_rv32i.cpp
        ${SRC_DIR}/rv32i/mem_rv32i.h
        ${SRC_DIR}/rv32i/emulator_api.cpp
        ${SRC_DIR}/obf/restore.cpp
        ${SRC_DIR}/obf/restore.h
        ${COMMON_SOURCES}
)

target_compile_options(emulator PRIVATE ${NATIVE_CXX_FLAGS} -fPIC)
set_target_properties(emulator PROPERTIES
        OUTPUT_NAME "emulator"
        PREFIX ""
        SUFFIX ".so"
)

target_compile_options(emulator_static PRIVATE ${NATIVE_CXX_FLAGS})
set_target_properties(emulator_static PROPERTIES
        OUTPUT_NAME "emulator_static"
        PREFIX "lib"
        SUFFIX ".a"
)

# dist target for final build
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/dist
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:execrv32i> ${CMAKE_BINARY_DIR}/dist/execrv32i
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:emulator_static> ${CMAKE_BINARY_DIR}/dist/libemulator_static.a
    COMMAND ${CMAKE_COMMAND} -E copy ${SRC_DIR}/rv32i/emulator_api.h ${CMAKE_BINARY_DIR}/dist/emulator_api.h
    COMMAND ${CMAKE_COMMAND} -E copy ${SRC_DIR}/obf/gen_trampoline.py ${CMAKE_BINARY_DIR}/dist/gen_trampoline.py
    COMMAND ${CMAKE_COMMAND} -E copy ${SRC_DIR}/obf/obfuscate.py ${CMAKE_BINARY_DIR}/dist/obfuscate.py
    COMMAND ${CMAKE_COMMAND} -E copy ${SRC_DIR}/obf/CMakeLists.txt.template ${CMAKE_BINARY_DIR}/dist/CMakeLists.txt.template
    COMMAND chmod +x ${CMAKE_BINARY_DIR}/dist/obfuscate.py
    COMMAND chmod +x ${CMAKE_BINARY_DIR}/dist/gen_trampoline.py
    DEPENDS execrv32i emulator_static
    COMMENT "Packaging tools to ${CMAKE_BINARY_DIR}/dist"
)

# execrv32i: disassembler, emulator, obfuscator
add_executable(execrv32i
        main.cpp
        ${SRC_DIR}/rv32i/dis_rv32i.cpp
        ${SRC_DIR}/rv32i/cpu_rv32i.cpp
        ${SRC_DIR}/rv32i/mem_rv32i.cpp
        ${SRC_DIR}/obf/obfuscate.cpp
        ${SRC_DIR}/obf/restore.cpp
        src/rv32i/regs_rv32i.h
)

target_link_libraries(execrv32i PRIVATE emulator ${CMAKE_DL_LIBS})
target_compile_options(execrv32i PRIVATE ${NATIVE_CXX_FLAGS})

# summary:
message(STATUS "=== Build Configuration ===")
message(STATUS "Native C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Native C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "RISC-V C Compiler: ${RISCV_C_COMPILER}")
message(STATUS "RISC-V C++ Compiler: ${RISCV_CXX_COMPILER}")
message(STATUS "RISC-V Architecture: ${RISCV_ARCH}")
message(STATUS "RISC-V ABI: ${RISCV_ABI}")
message(STATUS "Output Directory: ${OUTPUT_DIR}")
message(STATUS "")
message(STATUS "Build Targets:")
message(STATUS "  Native: emulator.so, execrv32i (unified disassembler + emulator)")
message(STATUS "  RISC-V: riscv_test")
message(STATUS "============================")
