#ifndef EMULATOR_API_H
#define EMULATOR_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" { // Has to be C callable since the target programs are C
#endif

// Execute RV32I bytecode with the given arguments
// Returns the value in a0
uint32_t rv32i_call(const uint8_t* bytecode, size_t size, ...);

// Execute RV32I bytecode with the given arguments
// Returns the value in a0 (low) and a1 (high) combined
uint64_t rv32i_call64(const uint8_t* bytecode, size_t size, ...);

#ifdef __cplusplus
}
#endif

#endif // EMULATOR_API_H
