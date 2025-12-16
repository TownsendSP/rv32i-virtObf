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
set(TARGET_DIR "${CMAKE_SOURCE_DIR}/target")
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")

file(MAKE_DIRECTORY ${OUTPUT_DIR})

# Common Code
set(COMMON_SOURCES
        ${SRC_DIR}/rv32i/dis_rv32i.cpp
        ${SRC_DIR}/rv32i/dis_rv32i.h
)


# Emulator Library (Shared)
add_library(emulator SHARED
        ${SRC_DIR}/rv32i/cpu_rv32i.cpp
        ${SRC_DIR}/rv32i/cpu_rv32i.h
        ${SRC_DIR}/rv32i/mem_rv32i.cpp
        ${SRC_DIR}/rv32i/mem_rv32i.h
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

# Emulator Library (Static)
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

target_compile_options(emulator_static PRIVATE ${NATIVE_CXX_FLAGS})
set_target_properties(emulator_static PROPERTIES
        OUTPUT_NAME "emulator_static"
        PREFIX "lib"
        SUFFIX ".a"
)

# ============================================================================
# DIST TARGET
# ============================================================================
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

# ============================================================================
# TARGET 3: EXECRV32I - Unified Disassembler and Emulator (Native x64)
# ============================================================================
# This executable provides both disassembly and emulation capabilities
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

# Function to compile a C source file to rv32i and extract binary
function(add_riscv_function TARGET_NAME SOURCE_FILE)
    set(OBJ_FILE "${OUTPUT_DIR}/${TARGET_NAME}.o")
    set(ELF_FILE "${OUTPUT_DIR}/${TARGET_NAME}.elf")
    set(BIN_FILE "${OUTPUT_DIR}/${TARGET_NAME}.rv32i")
    set(ASM_FILE "${OUTPUT_DIR}/${TARGET_NAME}.asm")

    # Step 1: Compile to object file
    add_custom_command(
            OUTPUT ${OBJ_FILE}
            COMMAND ${RISCV_C_COMPILER}
            ${RISCV_C_FLAGS}
            -c ${SOURCE_FILE}
            -o ${OBJ_FILE}
            DEPENDS ${SOURCE_FILE}
            COMMENT "Compiling ${TARGET_NAME} to RISC-V bare-metal object file"
            VERBATIM
    )

    # Step 2: Link to ELF
    add_custom_command(
            OUTPUT ${ELF_FILE}
            COMMAND ${RISCV_C_COMPILER}
            ${RISCV_C_FLAGS}
            ${RISCV_LINK_FLAGS}
            ${OBJ_FILE}
            -o ${ELF_FILE}
            DEPENDS ${OBJ_FILE}
            COMMENT "Linking ${TARGET_NAME} to RISC-V bare-metal ELF"
            VERBATIM
    )

    # Step 3: Extract .text section as raw binary
    add_custom_command(
            OUTPUT ${BIN_FILE}
            COMMAND ${RISCV_OBJCOPY}
            -O binary
            --only-section=.text
            ${ELF_FILE}
            ${BIN_FILE}
            DEPENDS ${ELF_FILE}
            COMMENT "Extracting .text section to ${TARGET_NAME}.rv32i"
            VERBATIM
    )

    # Step 4: Generate assembly listing (for debugging/verification)
    add_custom_command(
            OUTPUT ${ASM_FILE}
            COMMAND ${RISCV_OBJDUMP}
            -d ${ELF_FILE}
            > ${ASM_FILE}
            DEPENDS ${ELF_FILE}
            COMMENT "Generating disassembly listing for ${TARGET_NAME}"
            VERBATIM
    )

    # Create a target that depends on all outputs
    add_custom_target(${TARGET_NAME} ALL
            DEPENDS ${OBJ_FILE} ${ELF_FILE} ${BIN_FILE} ${ASM_FILE}
    )
endfunction()

# Add your virtualized functions here
# Example: add_riscv_function(doOperation "${TARGET_DIR}/addition.c")

add_riscv_function(riscv_test "${TARGET_DIR}/riscv-test.c")
add_riscv_function(doOperation "${TARGET_DIR}/doOperation.c")

# ============================================================================
# CONVENIENCE TARGETS
# ============================================================================

# Build all native (x64) components
add_custom_target(native ALL
        DEPENDS emulator execrv32i
)

# Build all RISC-V components
add_custom_target(riscv ALL
        DEPENDS riscv_test doOperation
)

# Build everything
add_custom_target(all_targets ALL
        DEPENDS native riscv
)

# ============================================================================
# INSTALLATION (Optional)
# ============================================================================
install(TARGETS execrv32i emulator
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
)

install(FILES
        ${OUTPUT_DIR}/riscv_test.rv32i
        ${OUTPUT_DIR}/riscv_test.elf
        ${OUTPUT_DIR}/riscv_test.asm
        DESTINATION share/rv32i-obfuscator
        OPTIONAL
)

# ============================================================================
# ============================================================================
# SUMMARY
# ============================================================================
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
