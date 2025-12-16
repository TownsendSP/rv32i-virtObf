#include "restore.h"
#include <algorithm>
#include <stdexcept>

void deobfuscate(std::vector<uint8_t>& data) {
    if (data.size() % 4 != 0) {
        throw std::runtime_error("Data size must be a multiple of 4 bytes for restoration");
    }

    std::reverse(data.begin(), data.end());

    uint32_t key = 0xDEADBEEF;
    for (size_t i = 0; i < data.size(); i += 4) {
        uint32_t word = data[i] | (data[i+1] << 8) | (data[i+2] << 16) | (data[i+3] << 24);
        word ^= key;

        data[i] = word & 0xFF;
        data[i+1] = (word >> 8) & 0xFF;
        data[i+2] = (word >> 16) & 0xFF;
        data[i+3] = (word >> 24) & 0xFF;
    }
}
