// ======================================================================== //
// Copyright 2020-2021 Michel Fäh
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ======================================================================== //

#include "../error.hpp"
#include "instructions.hpp"

uint32_t instr_add_ireg_ireg(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_A_OFFSET = 2;
    constexpr uint32_t IREG_B_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t iRegA = vm->MMU.InstrBuffer[IREG_A_OFFSET];
    uint8_t iRegB = vm->MMU.InstrBuffer[IREG_B_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
    }

    IntVal iRegAVal;
    IntVal iRegBVal;

    if (vm->MMU.getIntReg(iRegA, iRegAVal) != 0) {
        return 0xFF;
    }

    if (vm->MMU.getIntReg(iRegB, iRegBVal) != 0) {
        return 0xFF;
    }

    IntVal result;
    switch (intType) {
    case IntType::I8:
        result.I8 = iRegAVal.I8 + iRegBVal.I8;
        break;
    case IntType::I16:
        result.I16 = iRegAVal.I16 + iRegBVal.I16;
        break;
    case IntType::I32:
        result.I32 = iRegAVal.I32 + iRegBVal.I32;
        break;
    case IntType::I64:
        result.I64 = iRegAVal.I64 + iRegBVal.I64;
        break;
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setIntReg(iRegB, result, intType);

    return UVM_SUCCESS;
}

uint32_t instr_sub_ireg_ireg(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_A_OFFSET = 2;
    constexpr uint32_t IREG_B_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t iRegA = vm->MMU.InstrBuffer[IREG_A_OFFSET];
    uint8_t iRegB = vm->MMU.InstrBuffer[IREG_B_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
    }

    IntVal iRegAVal;
    IntVal iRegBVal;

    if (vm->MMU.getIntReg(iRegA, iRegAVal) != 0) {
        return 0xFF;
    }

    if (vm->MMU.getIntReg(iRegB, iRegBVal) != 0) {
        return 0xFF;
    }

    IntVal result;
    switch (intType) {
    case IntType::I8:
        result.I8 = iRegAVal.I8 - iRegBVal.I8;
        break;
    case IntType::I16:
        result.I16 = iRegAVal.I16 - iRegBVal.I16;
        break;
    case IntType::I32:
        result.I32 = iRegAVal.I32 - iRegBVal.I32;
        break;
    case IntType::I64:
        result.I64 = iRegAVal.I64 - iRegBVal.I64;
        break;
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setIntReg(iRegB, result, intType);

    return UVM_SUCCESS;
}

uint32_t instr_mul_ireg_ireg(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_A_OFFSET = 2;
    constexpr uint32_t IREG_B_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t iRegA = vm->MMU.InstrBuffer[IREG_A_OFFSET];
    uint8_t iRegB = vm->MMU.InstrBuffer[IREG_B_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
    }

    IntVal iRegAVal;
    IntVal iRegBVal;

    if (vm->MMU.getIntReg(iRegA, iRegAVal) != 0) {
        return 0xFF;
    }

    if (vm->MMU.getIntReg(iRegB, iRegBVal) != 0) {
        return 0xFF;
    }

    IntVal result;
    switch (intType) {
    case IntType::I8:
        result.I8 = iRegAVal.I8 * iRegBVal.I8;
        break;
    case IntType::I16:
        result.I16 = iRegAVal.I16 * iRegBVal.I16;
        break;
    case IntType::I32:
        result.I32 = iRegAVal.I32 * iRegBVal.I32;
        break;
    case IntType::I64:
        result.I64 = iRegAVal.I64 * iRegBVal.I64;
        break;
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setIntReg(iRegB, result, intType);

    return UVM_SUCCESS;
}

uint32_t instr_div_ireg_ireg(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_A_OFFSET = 2;
    constexpr uint32_t IREG_B_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t iRegA = vm->MMU.InstrBuffer[IREG_A_OFFSET];
    uint8_t iRegB = vm->MMU.InstrBuffer[IREG_B_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
    }

    IntVal iRegAVal;
    IntVal iRegBVal;

    if (vm->MMU.getIntReg(iRegA, iRegAVal) != 0) {
        return 0xFF;
    }

    if (vm->MMU.getIntReg(iRegB, iRegBVal) != 0) {
        return 0xFF;
    }

    IntVal result;
    switch (intType) {
    case IntType::I8:
        result.I8 = iRegAVal.I8 / iRegBVal.I8;
        break;
    case IntType::I16:
        result.I16 = iRegAVal.I16 / iRegBVal.I16;
        break;
    case IntType::I32:
        result.I32 = iRegAVal.I32 / iRegBVal.I32;
        break;
    case IntType::I64:
        result.I64 = iRegAVal.I64 / iRegBVal.I64;
        break;
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setIntReg(iRegB, result, intType);

    return UVM_SUCCESS;
}

uint32_t instr_unsigned_cast_to_long(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t IREG_OFFSET = 1;

    constexpr uint64_t I8_MASK = 0xF;
    constexpr uint64_t I16_MASK = 0xFF;
    constexpr uint64_t I32_MASK = 0xFFFF;

    uint8_t iReg = vm->MMU.InstrBuffer[IREG_OFFSET];

    IntVal iRegVal;
    if (vm->MMU.getIntReg(iReg, iRegVal) != 0) {
        return 0xFF;
    }

    IntType type = static_cast<IntType>(flag);
    switch (type) {
    case IntType::I8:
        iRegVal.I64 &= I8_MASK;
        break;
    case IntType::I16:
        iRegVal.I64 &= I16_MASK;
        break;
    case IntType::I32:
        iRegVal.I64 &= I32_MASK;
        break;
    }

    vm->MMU.setIntReg(iReg, iRegVal, IntType::I64);

    return UVM_SUCCESS;
}
