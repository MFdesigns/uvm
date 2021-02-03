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

/**
 * Performs operations for instructions add, sub, mul and div
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Type of INSTR_FLAG_OP_* flag
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SOURCE_REG, E_INVALID_TARGET_REG, E_DIVISON_ZERO]
 */
uint32_t
instr_arithm_common_int_ireg_ireg(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // add <iT> <iR1> <iR2>
    // sub<iT> <iR1> <iR2>
    // mul <iT> <iR1> <iR2>
    // div <iT> <iR1> <iR2>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t SRC_REG_OFFSET = 2;
    constexpr uint32_t DEST_REG_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t srcRegId = vm->MMU.InstrBuffer[SRC_REG_OFFSET];
    uint8_t destRegId = vm->MMU.InstrBuffer[DEST_REG_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return E_INVALID_TYPE;
    }

    IntVal srcRegVal;
    IntVal destRegVal;

    if (vm->MMU.getIntReg(srcRegId, srcRegVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }
    if (vm->MMU.getIntReg(destRegId, destRegVal) != 0) {
        return E_INVALID_TARGET_REG;
    }

    IntVal result;
    if ((flag & INSTR_FLAG_OP_ADD) != 0) {
        switch (intType) {
        case IntType::I8:
            result.I8 = srcRegVal.I8 + destRegVal.I8;
            break;
        case IntType::I16:
            result.I16 = srcRegVal.I16 + destRegVal.I16;
            break;
        case IntType::I32:
            result.I32 = srcRegVal.I32 + destRegVal.I32;
            break;
        case IntType::I64:
            result.I64 = srcRegVal.I64 + destRegVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_SUB) != 0) {
        switch (intType) {
        case IntType::I8:
            result.I8 = srcRegVal.I8 - destRegVal.I8;
            break;
        case IntType::I16:
            result.I16 = srcRegVal.I16 - destRegVal.I16;
            break;
        case IntType::I32:
            result.I32 = srcRegVal.I32 - destRegVal.I32;
            break;
        case IntType::I64:
            result.I64 = srcRegVal.I64 - destRegVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_MUL) != 0) {
        switch (intType) {
        case IntType::I8:
            result.I8 = srcRegVal.I8 * destRegVal.I8;
            break;
        case IntType::I16:
            result.I16 = srcRegVal.I16 * destRegVal.I16;
            break;
        case IntType::I32:
            result.I32 = srcRegVal.I32 * destRegVal.I32;
            break;
        case IntType::I64:
            result.I64 = srcRegVal.I64 * destRegVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_DIV) != 0) {
        switch (intType) {
        case IntType::I8:
            if (destRegVal.I8 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I8 = srcRegVal.I8 / destRegVal.I8;
            break;
        case IntType::I16:
            if (destRegVal.I16 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I16 = srcRegVal.I16 / destRegVal.I16;
            break;
        case IntType::I32:
            if (destRegVal.I32 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I32 = srcRegVal.I32 / destRegVal.I32;
            break;
        case IntType::I64:
            if (destRegVal.I64 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I64 = srcRegVal.I64 / destRegVal.I64;
            break;
        }
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setIntReg(destRegId, result, intType);

    return UVM_SUCCESS;
}

/**
 * Typecasts unsigned i8, i16 or i32 to i64
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag IntType determining instruction version
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_SOURCE_REG]
 */
uint32_t instr_unsigned_cast_to_long(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // b2l <iR>
    // s2l <iR>
    // i2l <iR>

    constexpr uint32_t IREG_OFFSET = 1;

    constexpr uint64_t I8_MASK = 0xF;
    constexpr uint64_t I16_MASK = 0xFF;
    constexpr uint64_t I32_MASK = 0xFFFF;

    uint8_t srcRegId = vm->MMU.InstrBuffer[IREG_OFFSET];

    IntVal srcRegVal;
    if (vm->MMU.getIntReg(srcRegId, srcRegVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }

    IntType type = static_cast<IntType>(flag);
    switch (type) {
    case IntType::I8:
        srcRegVal.I64 &= I8_MASK;
        break;
    case IntType::I16:
        srcRegVal.I64 &= I16_MASK;
        break;
    case IntType::I32:
        srcRegVal.I64 &= I32_MASK;
        break;
    }

    vm->MMU.setIntReg(srcRegId, srcRegVal, IntType::I64);

    return UVM_SUCCESS;
}
