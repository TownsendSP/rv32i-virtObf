#ifndef RESTORE_H
#define RESTORE_H

#include <vector>
#include <cstdint>

// Restore data: Reverse bytes then XOR with 0xDEADBEEF
void restore(std::vector<uint8_t>& data);

#endif // RESTORE_H
