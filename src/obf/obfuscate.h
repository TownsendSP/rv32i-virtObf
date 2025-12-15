#ifndef OBFUSCATE_H
#define OBFUSCATE_H

#include <vector>
#include <cstdint>

// Obfuscate data: XOR with 0xDEADBEEF (4-byte aligned) then reverse bytes
std::vector<uint8_t> obfuscate(const std::vector<uint8_t>& data);

#endif // OBFUSCATE_H
