#ifndef MEM_RV32I_H
#define MEM_RV32I_H

#include <vector>
#include <cstdint>
#include <algorithm>

// Memory layout for RV32I emulation
// This provides a simple, auto-growing memory system
class mem_rv32i {
private:
    // Static memory layout values - initialized in init()
    static uint32_t INITIAL_SIZE;
    static uint32_t CODE_START;
    static uint32_t DATA_START;
    static uint32_t HEAP_START;
    static uint32_t STACK_START;

    std::vector<uint8_t> memory;

    uint32_t code_base;      // Where code is loaded
    uint32_t code_size;      // Size of code section
    uint32_t stack_ptr;      // Current stack pointer (sp register = x2)
    uint32_t heap_ptr;       // Current heap break (for future malloc emulation)

    void ensure_capacity(uint32_t addr);

public:
    static void init();

    mem_rv32i();

    // Load code into memory at the code section
    void load_code(const std::vector<uint8_t>& code);

    // Byte access
    uint8_t read8(uint32_t addr);
    void write8(uint32_t addr, uint8_t val);

    // Half-word access (16-bit)
    uint16_t read16(uint32_t addr);
    void write16(uint32_t addr, uint16_t val);

    // Word access (32-bit) - little-endian
    uint32_t read32(uint32_t addr);
    void write32(uint32_t addr, uint32_t val);

    // Getters/setters for special addresses
    uint32_t get_code_base() const { return code_base; }
    uint32_t get_code_size() const { return code_size; }
    uint32_t get_stack_ptr() const { return stack_ptr; }
    void set_stack_ptr(uint32_t sp) { stack_ptr = sp; }
    uint32_t get_heap_ptr() const { return heap_ptr; }

    // Debug info
    size_t get_memory_size() const { return memory.size(); }
};
#endif //MEM_RV32I_H
