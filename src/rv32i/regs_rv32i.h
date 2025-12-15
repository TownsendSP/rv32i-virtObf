//
// Created by tgsp on 12/15/25.
//

#ifndef REGS_RV32I_H
#define REGS_RV32I_H

#include <cstring>

// Register number to ABI name lookup (indexed by register number 0-31)
static const char* riscv_abi_names[32] = {
    "zero",  // x0
    "ra",    // x1
    "sp",    // x2
    "gp",    // x3
    "tp",    // x4
    "t0",    // x5
    "t1",    // x6
    "t2",    // x7
    "s0",    // x8  (fp)
    "s1",    // x9
    "a0",    // x10
    "a1",    // x11
    "a2",    // x12
    "a3",    // x13
    "a4",    // x14
    "a5",    // x15
    "a6",    // x16
    "a7",    // x17
    "s2",    // x18
    "s3",    // x19
    "s4",    // x20
    "s5",    // x21
    "s6",    // x22
    "s7",    // x23
    "s8",    // x24
    "s9",    // x25
    "s10",   // x26
    "s11",   // x27
    "t3",    // x28
    "t4",    // x29
    "t5",    // x30
    "t6"     // x31
};

// Structure for reverse lookup (ABI name to register number)
typedef struct {
    const char* abi_name;
    int reg_num;
} riscv_reg_map_t;

static const riscv_reg_map_t riscv_reg_map[] = {
    {"zero", 0},  {"ra", 1},    {"sp", 2},    {"gp", 3},
    {"tp", 4},    {"t0", 5},    {"t1", 6},    {"t2", 7},
    {"s0", 8},    {"fp", 8},    {"s1", 9},    {"a0", 10},
    {"a1", 11},   {"a2", 12},   {"a3", 13},   {"a4", 14},
    {"a5", 15},   {"a6", 16},   {"a7", 17},   {"s2", 18},
    {"s3", 19},   {"s4", 20},   {"s5", 21},   {"s6", 22},
    {"s7", 23},   {"s8", 24},   {"s9", 25},   {"s10", 26},
    {"s11", 27},  {"t3", 28},   {"t4", 29},   {"t5", 30},
    {"t6", 31}
};

#define RISCV_REG_MAP_SIZE (sizeof(riscv_reg_map) / sizeof(riscv_reg_map[0]))

// Helper function to get register number from ABI name
static inline int riscv_abi_to_reg(const char* abi_name) {
    for (int i = 0; i < RISCV_REG_MAP_SIZE; i++) {
        if (strcmp(riscv_reg_map[i].abi_name, abi_name) == 0) {
            return riscv_reg_map[i].reg_num;
        }
    }
    return -1; // Not found
}

// Helper function to get ABI name from register number
static inline const char* riscv_reg_to_abi(int reg_num) {
    if (reg_num >= 0 && reg_num < 32) {
        return riscv_abi_names[reg_num];
    }
    return NULL;
}

#endif //REGS_RV32I_H
