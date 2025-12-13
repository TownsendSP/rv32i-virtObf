#include "vmcore.h"

vmcore::vmcore(): pc(0) {
    // Initialize all registers to 0
    for (int i = 0; i < 32; i++) {
        registers[i] = 0;
    }
    // Set stack pointer to top of stack
    registers[2] = memory.get_stack_ptr();  // sp = x2
}

void vmcore::load_program(const std::vector<uint8_t> &program) {
    memory.load_code(program);
    pc = memory.get_code_base();  // Start execution at beginning of code
}

uint32_t vmcore::read_reg(uint8_t reg) const {
    if (reg == 0) return 0;  // x0 is always 0
    return registers[reg];
}

void vmcore::write_reg(uint8_t reg, uint32_t value) {
    if (reg == 0) return;  // Writing to x0 is ignored
    registers[reg] = value;
}

uint32_t vmcore::fetch() {
    uint32_t inst = memory.read32(pc);
    return inst;
}

void vmcore::advance_pc() {
    pc += 4;
}

void vmcore::branch(int32_t offset) {
    pc += offset;
}

void vmcore::jump(uint32_t target) {
    pc = target;
}
