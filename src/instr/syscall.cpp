// ======================================================================== //
// Copyright 2020 Michel Fäh
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

#include "syscall.hpp"
#include <iostream>

/**
 * Takes the syscall arguments in register r0-r15 and prints to console
 * @param vm Current UVM instance
 * @return On success return true otherwise false
 */
bool internalPrint(UVM* vm) {
    // r0 contains string pointer
    // r1 contains string length
    IntVal r0;
    IntVal r1;
    vm->MMU.getIntReg(0x05, r0);
    vm->MMU.getIntReg(0x06, r1);

    uint8_t* buff = nullptr;
    if (!vm->MMU.readPhysicalMem(r0.I64, r1.I32, PERM_READ_MASK, &buff)) {
        return false;
    }

    std::string tmpStr{(char*)buff, r1.I32};
    std::cout << tmpStr;

    return true;
}

/**
 * Performs the corresponding syscall
 * @param vm Current UVM instance
 * @return On success return true otherwise false
 */
bool Instr::syscall(UVM* vm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, 2, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    constexpr uint8_t SYS_PRINT = 0x1;
    constexpr uint8_t SYS_ALLOC = 0x41;
    constexpr uint8_t SYS_DEALLOC = 0x44;
    constexpr uint8_t REG_R0 = 0x5;

    uint8_t syscallType = buff[1];
    bool callSuccess = true;
    switch (syscallType) {
    case SYS_PRINT:
        callSuccess = internalPrint(vm);
        break;
    case SYS_ALLOC: {
        IntVal allocSize;
        if (vm->MMU.getIntReg(REG_R0, allocSize) != 0) {
            return false;
        }

        IntVal allocAddr;
        allocAddr.I64 = vm->MMU.allocHeap(allocSize.I32);

        if (allocAddr.I64 == UVM_NULLPTR) {
            return false;
        }

        if (vm->MMU.setIntReg(REG_R0, allocAddr, IntType::I64) != 0) {
            return false;
        }
    } break;
    case SYS_DEALLOC:
        // TODO: Implement dealloc
        break;
    default:
        return false;
    }

    if (!callSuccess) {
        return false;
    }

    return true;
}
