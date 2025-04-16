

#include "dis_rv32i.h"

#include <stdexcept>
#include <memory>
#include <stdexcept>

/*
 * U-Type: 0110111, 0010111
 * I-
* I:  1100111  0000011  0010011
 * S: 0100011
 * R-Shift: 0010011
 * R-type: 0110011
 * B: 1100011
 * J: 1101111
 * Fence: 0001111
 * Ecall: 1110011
 *
 */



Instruction::Instruction(uint32_t raw) : raw(raw) {
    opcode = raw & 0x7F;
    uint32_t raw2 = raw;
    switch (opcode) {
        case 0b1100111:
            //I-type
            type = I_TYPE;
            break;
        case 0b0000011:
            //I-type
            type = I_TYPE;
            break;
        case 0b0010011:
            //I-type
            type = I_TYPE;
            break;
        case 0b0110111:
            //U:
            type = U_TYPE;
            break;
        case 0b0010111:
            //U:
            type = U_TYPE;
            break;
        case 0b0100011:
            //S:
            type = S_TYPE;
            break;
        // case 0b0010011:
        //        //Rs:
        //            type = RSHIFT_TYPE;
        //    break;
        case 0b0110011:
            //Rt:
            type = R_TYPE;
            break;
        case 0b1100011:
            //B:
            type = B_TYPE;
            break;
        case 0b1101111:
            //J:
            type = J_TYPE;
            break;
        case 0b0001111:
            //Fence:
            type = FENCE_TYPE;
            break;
        case 0b1110011:
            //Ecall:
            type = SYS_TYPE;
            break;
        default:
            throw std::invalid_argument("Unknown opcode");
    }


}

std::unique_ptr<Instruction> Instruction::create(uint32_t raw) {
    uint32_t opcode = raw & 0x7F;
    switch (opcode) {
        case 0b1100111:
            return std::make_unique<IType>(raw);
        case 0b0000011:
            return std::make_unique<IType>(raw);
        case 0b0010011:
            return std::make_unique<IType>(raw);
        case 0b0110111:
            return std::make_unique<UType>(raw);
        case 0b0010111:
            return std::make_unique<UType>(raw);
        case 0b0100011:
            return std::make_unique<SType>(raw);
        case 0b0110011:
            return std::make_unique<RType>(raw);
        case 0b1100011:
            return std::make_unique<BType>(raw);
        case 0b1101111:
            return std::make_unique<JType>(raw);
        case 0b0001111:
            return std::make_unique<FenceType>(raw);
        case 0b1110011:
            return std::make_unique<SysType>(raw);
        default:
            throw std::invalid_argument("Unknown opcode");
    }
}

IType::IType(uint32_t raw) : Instruction(raw) {

}
SType::SType(uint32_t raw) : Instruction(raw) {

}
RType::RType(uint32_t raw) : Instruction(raw) {

}
BType::BType(uint32_t raw) : Instruction(raw) {

}
JType::JType(uint32_t raw) : Instruction(raw) {

}
FenceType::FenceType(uint32_t raw) : Instruction(raw) {

}
SysType::SysType(uint32_t raw) : Instruction(raw) {


}