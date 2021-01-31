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
#include <memory>

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

    uint32_t allocSize = r1.I32;

    std::unique_ptr<uint8_t[]> buff = std::make_unique<uint8_t[]>(allocSize);

    uint32_t readRes = vm->MMU.readBig(r0.I64, buff.get(), allocSize, 0);
    if (readRes != UVM_SUCCESS) {
        return false;
    }

    std::string tmpStr{(char*)buff.get(), allocSize};
    switch (vm->Mode) {
    case ExecutionMode::USER:
        std::cout << tmpStr;
        break;
    case ExecutionMode::DEBUGGER:
        vm->DbgConsole << tmpStr;
        break;
    }

    return true;
}

/**
 * Performs the corresponding syscall
 * @param vm Current UVM instance
 * @return On success return true otherwise false
 */
uint32_t Instr::syscall(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t SYS_TYPE_OFFSET = 1;

    constexpr uint8_t SYS_PRINT = 0x1;
    constexpr uint8_t SYS_ALLOC = 0x41;
    constexpr uint8_t SYS_DEALLOC = 0x44;
    constexpr uint8_t REG_R0 = 0x5;

    uint8_t syscallType = vm->MMU.InstrBuffer[SYS_TYPE_OFFSET];
    bool callSuccess = true;
    switch (syscallType) {
    case SYS_PRINT:
        callSuccess = internalPrint(vm);
        break;
    case SYS_ALLOC: {
        IntVal allocSize;
        if (vm->MMU.getIntReg(REG_R0, allocSize) != 0) {
            return 0xFF;
        }

        IntVal allocAddr;
        allocAddr.I64 = vm->MMU.allocHeap(allocSize.I32);

        if (allocAddr.I64 == UVM_NULLPTR) {
            return 0xFF;
        }

        if (vm->MMU.setIntReg(REG_R0, allocAddr, IntType::I64) != 0) {
            return 0xFF;
        }
    } break;
    case SYS_DEALLOC: {
        IntVal vAddr;
        if (vm->MMU.getIntReg(REG_R0, vAddr) != 0) {
            return 0xFF;
        }

        uint32_t deallocRes = vm->MMU.deallocHeap(vAddr.I64);
        if (deallocRes != UVM_SUCCESS) {
            return 0xFF;
        }
    } break;
    default:
        return 0xFF;
    }

    if (!callSuccess) {
        return 0xFF;
    }

    return UVM_SUCCESS;
}
