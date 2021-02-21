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
#include <iostream>

/**
 * Compares two integer registers by subtraction and sets flags
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SRC_REG, E_INVALID_DEST_REG]
 */
uint32_t instr_cmp(UVM* vm, uint32_t width, uint32_t flag) {
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
        return E_INVALID_SRC_REG;
    }

    if (vm->MMU.getIntReg(destRegId, destRegVal) != 0) {
        return E_INVALID_DEST_REG;
    }

    IntVal result;
    uint32_t shiftWidth = 0;
    switch (intType) {
    case IntType::I8:
        result.I8 = srcRegVal.I8 - destRegVal.I8;
        shiftWidth = 7;
        break;
    case IntType::I16:
        result.I16 = srcRegVal.I16 - destRegVal.I16;
        shiftWidth = 15;
        break;
    case IntType::I32:
        result.I32 = srcRegVal.I32 - destRegVal.I32;
        shiftWidth = 31;
        break;
    case IntType::I64:
        result.I64 = srcRegVal.I64 - destRegVal.I64;
        shiftWidth = 63;
        break;
    }

    if (result.I64 == 0.0f) {
        vm->MMU.Flags.Zero = true;
    } else {
        vm->MMU.Flags.Zero = false;
    }

    if ((result.I64 >> shiftWidth) == 1) {
        vm->MMU.Flags.Signed = true;
    } else {
        vm->MMU.Flags.Signed = false;
    }

    return UVM_SUCCESS;
}

/**
 * Compares two float registers by subtraction and sets flags
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_SRC_REG, E_INVALID_DEST_REG]
 */
uint32_t instr_cmpf(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t SRC_REG_OFFSET = 2;
    constexpr uint32_t DEST_REG_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t srcRegId = vm->MMU.InstrBuffer[SRC_REG_OFFSET];
    uint8_t destRegId = vm->MMU.InstrBuffer[DEST_REG_OFFSET];

    FloatType floatType = FloatType::F32;
    if (!parseFloatType(type, &floatType)) {
        return E_INVALID_TYPE;
    }

    FloatVal srcRegVal;
    FloatVal destRegVal;

    if (vm->MMU.getFloatReg(srcRegId, srcRegVal) != 0) {
        return E_INVALID_SRC_REG;
    }

    if (vm->MMU.getFloatReg(destRegId, destRegVal) != 0) {
        return E_INVALID_DEST_REG;
    }

    FloatVal result;
    // This bit mask is used to determin if the float/double is signed
    uint64_t floatSignBitMask = 0;
    switch (floatType) {
    case FloatType::F32:
        result.F32 = srcRegVal.F32 - destRegVal.F32;
        floatSignBitMask = 0x80000000;
        break;
    case FloatType::F64:
        result.F64 = srcRegVal.F64 - destRegVal.F64;
        floatSignBitMask = 0x8000000000000000;
        break;
    }

    if (result.F64 == 0) {
        vm->MMU.Flags.Zero = true;
    } else {
        vm->MMU.Flags.Zero = false;
    }

    if ((*reinterpret_cast<uint64_t*>(&result.F64) & floatSignBitMask) == 0) {
        vm->MMU.Flags.Signed = false;
    } else {
        vm->MMU.Flags.Signed = true;
    }

    return UVM_SUCCESS;
}

/**
 * Checks what flags are set and performs different jumps on valid flags
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Type of JumpCondition
 * @return On success returns UVM_SUCCESS on success and jumped returns
 * UVM_SUCCESS_JUMPED otherwise error state [E_INVALID_JUMP_DEST,
 * E_MISSING_PERM, E_INVALID_SRC_REG, E_INVALID_DEST_REG]
 */
uint32_t instr_jmp(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t ADDR_OFFSET = 1;

    uint64_t targetAddr =
        *reinterpret_cast<uint64_t*>(&vm->MMU.InstrBuffer[ADDR_OFFSET]);

    MemSection* memSec = vm->MMU.findSection(targetAddr, 1);
    if (memSec == nullptr) {
        return E_INVALID_JUMP_DEST;
    }

    if ((memSec->Perm & PERM_EXE_MASK) != PERM_EXE_MASK) {
        return E_MISSING_PERM;
    }

    JumpCondition cond = static_cast<JumpCondition>(flag);
    switch (cond) {
    case JumpCondition::UNCONDITIONAL:
        vm->MMU.IP = targetAddr;
        return UVM_SUCCESS_JUMPED;
    case JumpCondition::IF_EQUALS:
        if (vm->MMU.Flags.Zero) {
            vm->MMU.IP = targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    case JumpCondition::IF_NOT_EQUALS:
        if (!vm->MMU.Flags.Zero) {
            vm->MMU.IP = targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    case JumpCondition::IF_GREATER_THAN:
        if (!vm->MMU.Flags.Zero && !vm->MMU.Flags.Signed) {
            vm->MMU.IP = targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    case JumpCondition::IF_LESS_THAN:
        if (!vm->MMU.Flags.Zero && vm->MMU.Flags.Signed) {
            vm->MMU.IP = targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    case JumpCondition::IF_GREATER_EQUALS:
        if (!vm->MMU.Flags.Signed) {
            vm->MMU.IP = targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    case JumpCondition::IF_LESS_EQUALS:
        if ((vm->MMU.Flags.Zero && !vm->MMU.Flags.Signed) ||
            (!vm->MMU.Flags.Zero && vm->MMU.Flags.Signed)) {
            vm->MMU.IP = targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    }

    return UVM_SUCCESS;
}
