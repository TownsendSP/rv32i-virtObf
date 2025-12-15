#include "obfuscate.h"
#include <algorithm>
#include <stdexcept>

std::vector<uint8_t> obfuscate(const std::vector<uint8_t>& data) {
    if (data.size() % 4 != 0) {
        throw std::runtime_error("Data size must be a multiple of 4 bytes for obfuscation");
    }

    std::vector<uint8_t> result = data;

    // 1. XOR with 0xDEADBEEF
    uint32_t key = 0xDEADBEEF;
    for (size_t i = 0; i < result.size(); i += 4) {
        // Read 32-bit word (little-endian assumption for simplicity in byte manipulation)
        uint32_t word = result[i] | (result[i+1] << 8) | (result[i+2] << 16) | (result[i+3] << 24);
        
        word ^= key;
        
        // Write back
        result[i] = word & 0xFF;
        result[i+1] = (word >> 8) & 0xFF;
        result[i+2] = (word >> 16) & 0xFF;
        result[i+3] = (word >> 24) & 0xFF;
    }

    // 2. Reverse bytes
    std::reverse(result.begin(), result.end());

    return result;
}
