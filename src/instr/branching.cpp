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

#include "branching.hpp"
#include "../error.hpp"
#include <iostream>

uint32_t Instr::cmpIRegToIReg(UVM* vm, uint32_t width, uint32_t flag) {
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
    uint32_t shiftWidth = 0;
    switch (intType) {
    case IntType::I8:
        result.I8 = iRegAVal.I8 - iRegBVal.I8;
        shiftWidth = 7;
        break;
    case IntType::I16:
        result.I16 = iRegAVal.I16 - iRegBVal.I16;
        shiftWidth = 15;
        break;
    case IntType::I32:
        result.I32 = iRegAVal.I32 - iRegBVal.I32;
        shiftWidth = 31;
        break;
    case IntType::I64:
        result.I64 = iRegAVal.I64 - iRegBVal.I64;
        shiftWidth = 63;
        break;
    }

    if (result.I64 == 0) {
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
 *
 * @return On success returns UVM_SUCCESS, if jmp was executed
 * UVM_SUCCESS_JUMPED otherwise an error status
 */
uint32_t Instr::jmp(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t ADDR_OFFSET = 1;
    // Get target vAddr
    uint64_t* targetAddr =
        reinterpret_cast<uint64_t*>(&vm->MMU.InstrBuffer[ADDR_OFFSET]);

    MemSection* memSec = vm->MMU.findSection(*targetAddr, 1);
    if (memSec == nullptr) {
        std::cout << "[Error] Jump to unknown memory address 0x" << std::hex
                  << *targetAddr << '\n';
        return 0xFF;
    }

    if ((memSec->Perm & PERM_EXE_MASK) != PERM_EXE_MASK) {
        std::cout << "[Error] Jump to section with missing executable "
                     "permisson at address 0x"
                  << std::hex << *targetAddr << '\n';
        return 0xFF;
    }

    JumpCondition cond = static_cast<JumpCondition>(flag);
    switch (cond) {
    case JumpCondition::UNCONDITIONAL:
        vm->MMU.IP = *targetAddr;
        return UVM_SUCCESS_JUMPED;
    case JumpCondition::IF_EQUALS:
        if (vm->MMU.Flags.Zero) {
            vm->MMU.IP = *targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    case JumpCondition::IF_NOT_EQUALS:
        if (!vm->MMU.Flags.Zero) {
            vm->MMU.IP = *targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    case JumpCondition::IF_GREATER_THAN:
        if (!vm->MMU.Flags.Zero && !vm->MMU.Flags.Signed) {
            vm->MMU.IP = *targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    case JumpCondition::IF_LESS_THAN:
        if (!vm->MMU.Flags.Zero && vm->MMU.Flags.Signed) {
            vm->MMU.IP = *targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    case JumpCondition::IF_GREATER_EQUALS:
        if (!vm->MMU.Flags.Signed) {
            vm->MMU.IP = *targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    case JumpCondition::IF_LESS_EQUALS:
        if ((vm->MMU.Flags.Zero && !vm->MMU.Flags.Signed) ||
            (!vm->MMU.Flags.Zero && vm->MMU.Flags.Signed)) {
            vm->MMU.IP = *targetAddr;
            return UVM_SUCCESS_JUMPED;
        }
        break;
    }

    return UVM_SUCCESS;
}
