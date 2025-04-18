#ifndef DISASSEMBLER_TEST_H
#define DISASSEMBLER_TEST_H

#include <cstdint>

// U-Type tests
int test_UType();

uint8_t test_LUI();
uint8_t test_AUIPC();

// I-Type tests
int test_IType();

uint8_t test_JALR();
uint8_t test_LB();
uint8_t test_LH();
uint8_t test_LW();
uint8_t test_LBU();
uint8_t test_LHU();
uint8_t test_ADDI();
uint8_t test_SLTI();
uint8_t test_SLTIU();
uint8_t test_XORI();
uint8_t test_ORI();
uint8_t test_ANDI();

uint8_t test_SLLI();
uint8_t test_SRLI();
uint8_t test_SRAI();

// S-Type tests
int test_SType();

uint8_t test_SB();
uint8_t test_SH();
uint8_t test_SW();

// R-Type tests
int test_RType();

uint8_t test_ADD();
uint8_t test_SUB();
uint8_t test_SLL();
uint8_t test_SLT();
uint8_t test_SLTU();
uint8_t test_XOR();
uint8_t test_SRL();
uint8_t test_SRA();
uint8_t test_OR();
uint8_t test_AND();

// B-Type tests
int test_BType();

uint8_t test_BEQ();
uint8_t test_BNE();
uint8_t test_BLT();
uint8_t test_BGE();
uint8_t test_BLTU();
uint8_t test_BGEU();

// J-Type tests
int test_JType();

uint8_t test_JAL();

#endif // DISASSEMBLER_TEST_H
