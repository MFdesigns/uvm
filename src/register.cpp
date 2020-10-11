/**
 * Copyright 2020 Michel Fäh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "register.hpp"
#include <cstring>
#include <iostream>

UVMInt::UVMInt(IntType type, IntVal val) : Type(type), Val(val) {}

UVMFloat::UVMFloat(FloatType type, FloatVal val) : Type(type), Val(val) {}

bool parseIntType(uint8_t type, IntType* intType) {
    switch (type) {
    case 0x1:
        *intType = IntType::I8;
        break;
    case 0x2:
        *intType = IntType::I16;
        break;
    case 0x3:
        *intType = IntType::I32;
        break;
    case 0x4:
        *intType = IntType::I64;
        break;
    default:
        return false;
    }

    return true;
}

bool parseFloatType(uint8_t type, FloatType* floatType) {
    switch (type) {
    case 0xF0:
        *floatType = FloatType::F32;
        break;
    case 0xF1:
        *floatType = FloatType::F64;
        break;
    default:
        return false;
    }

    return true;
}

RegisterManager::RegisterManager() : GP(16), FP(16) {}

bool RegisterManager::validateIntRegAccess(uint8_t id,
                                           IntVal** targetReg) const {
    if (id >= REG_GP_START && id <= REG_GP_END) {
        uint8_t gpRegIndex = id - REG_GP_START;
        *targetReg = (IntVal*)&GP[gpRegIndex];
    } else if (id == REG_STACK_PTR) {
        *targetReg = (IntVal*)&SP;
    } else if (id == REG_BASE_PTR) {
        *targetReg = (IntVal*)&BP;

        // Following register are not accesible from bytecode and thus resolve
        // into a runtime error

        // TODO: Error message does not reflect read or write access
    } else if (id == REG_INSTR_PTR) {
        std::cout << "[Runtime] Access violation: tried to write into "
                     "instruction pointer\n";
        return false;
    } else if (id == REG_FLAGS) {
        std::cout << "[Runtime] Access violation: tried to write into flags "
                     "register\n";
        return false;
    } else {
        std::cout
            << "[Runtime] Tried to write into unknown integer type register\n";
        return false;
    }

    return true;
}

bool RegisterManager::validateFloatRegAccess(uint8_t id,
                                             FloatVal** targetReg) const {
    if (id >= REG_FP_START && id <= REG_FP_END) {
        uint8_t fpRegIndex = id - REG_FP_START;
        *targetReg = (FloatVal*)&FP[fpRegIndex];
    } else {
        std::cout << "[Runtime] Tried to access unknown float type register\n";
        return false;
    }

    return true;
}

bool RegisterManager::setIntReg(uint8_t id, UVMInt* val) {
    IntVal* targetReg = nullptr;
    if (!validateIntRegAccess(id, &targetReg)) {
        return false;
    }

    switch (val->Type) {
    case IntType::I8:
        targetReg->I8 = val->Val.I8;
        break;
    case IntType::I16:
        targetReg->I16 = val->Val.I16;
        break;
    case IntType::I32:
        targetReg->I32 = val->Val.I32;
        break;
    case IntType::I64:
        targetReg->I64 = val->Val.I64;
        break;
    }

    return true;
}

bool RegisterManager::getIntReg(uint8_t id,
                                IntType type,
                                IntVal* outVal) const {
    IntVal* targetReg = nullptr;
    if (!validateIntRegAccess(id, &targetReg)) {
        return false;
    }

    switch (type) {
    case IntType::I8:
        outVal->I8 = targetReg->I8;
        break;
    case IntType::I16:
        outVal->I16 = targetReg->I16;
        break;
    case IntType::I32:
        outVal->I32 = targetReg->I32;
        break;
    case IntType::I64:
        outVal->I64 = targetReg->I64;
        break;
    }

    return true;
}

bool RegisterManager::setFloatReg(uint8_t id, UVMFloat* val) {
    FloatVal* targetReg = nullptr;
    if (!validateFloatRegAccess(id, &targetReg)) {
        return false;
    }

    switch (val->Type) {
    case FloatType::F32:
        targetReg->F32 = val->Val.F32;
        break;
    case FloatType::F64:
        targetReg->F64 = val->Val.F64;
        break;
    }

    return true;

    return true;
}

bool RegisterManager::getFloatReg(uint8_t id,
                                  FloatType type,
                                  FloatVal* outVal) const {
    FloatVal* targetReg = nullptr;
    if (!validateFloatRegAccess(id, &targetReg)) {
        return false;
    }

    switch (type) {
    case FloatType::F32:
        outVal->F32 = targetReg->F32;
        break;
    case FloatType::F64:
        outVal->F64 = targetReg->F64;
        break;
    }

    return true;
}

bool RegisterManager::evalRegOffset(uint8_t* buff, uint64_t* address) const {
    constexpr uint8_t RO_IR = 0x40;          // <iR>
    constexpr uint8_t RO_IR_P_I32 = 0x50;    // <iR> + <i32>
    constexpr uint8_t RO_IR_M_I32 = 0x70;    // <iR> - <i32>
    constexpr uint8_t RO_IR_P_IR_I16 = 0x46; // <iR1> + <iR2> * <i16>
    constexpr uint8_t RO_IR_M_IR_I16 = 0x66; // <iR1> + <iR2> * <i16>

    uint8_t layout = buff[0];
    uint8_t iRegA = buff[1];

    // First register in register offset can only be ip, sp, bp or r0-r15
    uint64_t iRegAVal = 0;
    if (!validateRegOffsetReg(iRegA, &iRegAVal)) {
        std::cout << "[Runtime] Invalid register in register offset "
                  << std::hex << iRegA << " (iR1)\n";
        return false;
    }

    // Extract possible i16 and i32 values
    uint32_t imm32 = 0;
    uint16_t imm16 = 0;
    std::memcpy(&imm32, &buff[2], 4);
    std::memcpy(&imm16, &buff[3], 2);

    // Calculate register offset address
    if (layout == RO_IR) {
        *address = iRegAVal;
    } else if (layout == RO_IR_P_I32) {
        *address = iRegAVal + imm32;
    } else if (layout == RO_IR_M_I32) {
        *address = iRegAVal - imm32;
    } else if (layout == RO_IR_P_IR_I16 || layout == RO_IR_M_IR_I16) {
        uint8_t iRegB = buff[2];
        uint64_t iRegBVal = 0;
        if (!validateRegOffsetReg(iRegB, &iRegBVal)) {
            std::cout << "[Runtime] Invalid register in register offset "
                      << std::hex << iRegB << " (iR2)\n";
            return false;
        }

        if (layout == RO_IR_P_IR_I16) {
            *address = iRegAVal + iRegBVal * imm16;
        } else {
            *address = iRegAVal - iRegBVal * imm16;
        }
    } else {
        return false;
    }

    return true;
}

bool RegisterManager::validateRegOffsetReg(uint8_t id, uint64_t* val) const {
    if (id >= REG_GP_START && id <= REG_GP_END) {
        *val = internalGetGP(id);
        return true;
    }

    switch (id) {
    case REG_INSTR_PTR:
        *val = internalGetIP();
        break;
    case REG_STACK_PTR:
        *val = internalGetSP();
        break;
    case REG_BASE_PTR:
        *val = internalGetBP();
        break;
    default:
        return false;
    }

    return true;
}

uint64_t RegisterManager::internalGetIP() const {
    return IP.I64;
}

uint64_t RegisterManager::internalGetSP() const {
    return SP.I64;
}

uint64_t RegisterManager::internalGetBP() const {
    return BP.I64;
}

uint64_t RegisterManager::internalGetGP(uint8_t id) const {
    return GP[id - REG_GP_START].I64;
}

void RegisterManager::internalSetIP(uint64_t val) {
    IP.I64 = val;
}

void RegisterManager::internalSetSP(uint64_t val) {
    SP.I64 = val;
}

void RegisterManager::internalSetBP(uint64_t val) {
    BP.I64 = val;
}
