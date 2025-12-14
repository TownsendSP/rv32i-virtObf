//
// Created by tgsp on 12/13/25.
//

#include "VirtMem.h"

// Static member variable initialization
uint32_t VirtualMemory::INITIAL_SIZE = 0;
uint32_t VirtualMemory::CODE_START = 0;
uint32_t VirtualMemory::DATA_START = 0;
uint32_t VirtualMemory::HEAP_START = 0;
uint32_t VirtualMemory::STACK_START = 0;

// Initialize static memory layout values
void VirtualMemory::init() {
    INITIAL_SIZE = 2 * 1024 * 1024;      // Start with 2MB
    CODE_START = 0x00010000;             // Code at 64KB
    DATA_START = 0x00100000;             // Data at 1MB
    HEAP_START = 0x01000000;             // Heap at 16MB
    STACK_START = 0x7fff0000;            // Stack at ~2GB, grows down
}

VirtualMemory::VirtualMemory()
    : memory(INITIAL_SIZE, 0)
    , code_base(CODE_START)
    , code_size(0)
    , stack_ptr(STACK_START)
    , heap_ptr(HEAP_START) {}

void VirtualMemory::ensure_capacity(uint32_t addr) {
    if (addr >= memory.size()) {
        // Grow memory as needed
        size_t new_size = std::max(static_cast<size_t>(addr) + 1, static_cast<size_t>(memory.size() * 2));
        memory.resize(new_size, 0);
    }
}

void VirtualMemory::load_code(const std::vector<uint8_t>& code) {
    code_size = code.size();
    ensure_capacity(code_base + code_size);
    std::copy(code.begin(), code.end(), memory.begin() + code_base);
}

uint8_t VirtualMemory::read8(uint32_t addr) {
    ensure_capacity(addr);
    return memory[addr];
}

void VirtualMemory::write8(uint32_t addr, uint8_t val) {
    ensure_capacity(addr);
    memory[addr] = val;
}

uint16_t VirtualMemory::read16(uint32_t addr) {
    ensure_capacity(addr + 1);
    return memory[addr] | (memory[addr+1] << 8);
}

void VirtualMemory::write16(uint32_t addr, uint16_t val) {
    ensure_capacity(addr + 1);
    memory[addr] = val & 0xFF;
    memory[addr+1] = (val >> 8) & 0xFF;
}

uint32_t VirtualMemory::read32(uint32_t addr) {
    ensure_capacity(addr + 3);
    return memory[addr] |
           (memory[addr+1] << 8) |
           (memory[addr+2] << 16) |
           (memory[addr+3] << 24);
}

void VirtualMemory::write32(uint32_t addr, uint32_t val) {
    ensure_capacity(addr + 3);
    memory[addr] = val & 0xFF;
    memory[addr+1] = (val >> 8) & 0xFF;
    memory[addr+2] = (val >> 16) & 0xFF;
    memory[addr+3] = (val >> 24) & 0xFF;
}