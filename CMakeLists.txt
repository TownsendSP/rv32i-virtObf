cmake_minimum_required(VERSION 3.20)
project(rv32i_VirtualizingObfuscator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# TOOLCHAIN CONFIGURATION
# ============================================================================
# Currently using native LLVM/Clang for both x64 and RISC-V cross-compilation
# Uncomment and modify these to use specific toolchain installations:

# --- Native x64 Toolchain (commented out - using system default) ---
# set(CMAKE_C_COMPILER "/usr/bin/clang")
# set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

# --- RISC-V Bare-Metal Toolchain ---
# Option 1: Use installed GNU RISC-V toolchain
# set(RISCV_TOOLCHAIN_PREFIX "/opt/riscv32-gnu-toolchain-elf-llvm-bin")
# set(RISCV_C_COMPILER "${RISCV_TOOLCHAIN_PREFIX}/bin/riscv32-unknown-elf-clang")
# set(RISCV_CXX_COMPILER "${RISCV_TOOLCHAIN_PREFIX}/bin/riscv32-unknown-elf-clang++")
# set(RISCV_OBJCOPY "${RISCV_TOOLCHAIN_PREFIX}/bin/riscv32-unknown-elf-objcopy")
# set(RISCV_OBJDUMP "${RISCV_TOOLCHAIN_PREFIX}/bin/riscv32-unknown-elf-objdump")

# Option 2: Use native LLVM with cross-compilation (CURRENT)
set(RISCV_C_COMPILER "clang")
set(RISCV_CXX_COMPILER "clang++")
set(RISCV_OBJCOPY "llvm-objcopy")
set(RISCV_OBJDUMP "llvm-objdump")

# RISC-V Architecture Configuration
set(RISCV_ARCH "rv32i" CACHE STRING "RISC-V architecture (rv32i, rv32im, etc.)")
set(RISCV_ABI "ilp32" CACHE STRING "RISC-V ABI (ilp32 for RV32I)")

# ============================================================================
# COMPILER FLAGS
# ============================================================================

# --- Native x64 flags ---
set(NATIVE_C_FLAGS -Wall -Wextra)
set(NATIVE_CXX_FLAGS -Wall -Wextra)

# --- RISC-V Bare-Metal flags (for production obfuscator) ---
set(RISCV_TARGET_FLAGS -target riscv32-unknown-elf -march=${RISCV_ARCH} -mabi=${RISCV_ABI})
set(RISCV_LINK_FLAGS -nostdlib -nostartfiles -static)
set(RISCV_C_FLAGS ${RISCV_TARGET_FLAGS} -O0 -Wall)
set(RISCV_CXX_FLAGS ${RISCV_TARGET_FLAGS} -O0 -Wall)

# ============================================================================
# DIRECTORIES
# ============================================================================
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(TARGET_DIR "${CMAKE_SOURCE_DIR}/target")
set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")

file(MAKE_DIRECTORY ${OUTPUT_DIR})

# ============================================================================
# COMMON/SHARED CODE (used by multiple targets)
# ============================================================================
set(COMMON_SOURCES
        ${SRC_DIR}/dis_rv32i.cpp
        ${SRC_DIR}/dis_rv32i.h
        ${SRC_DIR}/ISA_CONFIG.cpp
        ${SRC_DIR}/ISA_CONFIG.h
)

# ============================================================================
# TARGET 1: DISASSEMBLER (Native x64)
# ============================================================================
add_executable(disassembler
        ${SRC_DIR}/disassembler.cpp
        ${COMMON_SOURCES}
)

target_compile_options(disassembler PRIVATE ${NATIVE_CXX_FLAGS})

# ============================================================================
# TARGET 2: EMULATOR SHARED LIBRARY (Native x64)
# ============================================================================
add_library(emulator SHARED
        ${SRC_DIR}/vmcore.cpp
        ${SRC_DIR}/vmcore.h
        ${SRC_DIR}/cpu_rv32i.cpp
        ${SRC_DIR}/cpu_rv32i.h
        ${SRC_DIR}/VirtMem.cpp
        ${SRC_DIR}/VirtMem.h
        ${COMMON_SOURCES}

)

target_compile_options(emulator PRIVATE ${NATIVE_CXX_FLAGS} -fPIC)
set_target_properties(emulator PROPERTIES
        OUTPUT_NAME "emulator"
        PREFIX ""
        SUFFIX ".so"
)

# ============================================================================
# TARGET 3: EXECRV32I - Emulator Executor (Native x64)
# ============================================================================
# This executable loads emulator.so and executes a virtualized RV32I function
add_executable(execrv32i main.cpp src/dis_rv32i.cpp src/vmcore.cpp src/VirtMem.cpp src/ISA_CONFIG.cpp)

# Emulator target
add_executable(emu_rv32i src/emu_rv32i.cpp src/dis_rv32i.cpp src/vmcore.cpp src/VirtMem.cpp src/ISA_CONFIG.cpp)

target_link_libraries(execrv32i PRIVATE emulator ${CMAKE_DL_LIBS})
target_compile_options(execrv32i PRIVATE ${NATIVE_CXX_FLAGS})

# ============================================================================
# TARGET 4: RISC-V VIRTUALIZED FUNCTIONS (Bare-Metal Only)
# ============================================================================

# Function to compile a C source file to RISC-V bare-metal and extract binary
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
# Example: add_riscv_function(doOperation "${TARGET_DIR}/doOperation.c")

add_riscv_function(riscv_test "${TARGET_DIR}/riscv-test.c")
add_riscv_function(doOperation "${TARGET_DIR}/doOperation.c")

# ============================================================================
# CONVENIENCE TARGETS
# ============================================================================

# Build all native (x64) components
add_custom_target(native ALL
        DEPENDS disassembler emulator execrv32i
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
install(TARGETS disassembler execrv32i emulator
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
message(STATUS "  Native: disassembler, emulator.so, execrv32i")
message(STATUS "  RISC-V: riscv_test")
message(STATUS "============================")
