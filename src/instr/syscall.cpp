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

#include "syscall.hpp"
#include <iostream>

bool internalPrint(UVM* vm, RegisterManager* rm) {
    // r0 contains string pointer
    // r1 contains string length
    IntVal r0;
    IntVal r1;
    rm->getIntReg(0x05, IntType::I64, &r0);
    rm->getIntReg(0x06, IntType::I32, &r1);

    uint8_t* buff = nullptr;
    if (!vm->MMU.readPhysicalMem(r0.I64, r1.I32, PERM_READ_MASK, &buff)) {
        return false;
    }

    std::string tmpStr{(char*)buff, r1.I32};
    std::cout << tmpStr;

    return true;
}

bool Instr::syscall(UVM* vm, RegisterManager* rm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess = vm->MMU.readPhysicalMem(rm->internalGetIP(), 2, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    constexpr uint8_t SYS_PRINT = 0x1;

    uint8_t syscallType = buff[1];
    bool callSuccess = true;
    switch (syscallType) {
    case SYS_PRINT:
        callSuccess = internalPrint(vm, rm);
        break;
    default:
        return false;
    }

    if (!callSuccess) {
        return false;
    }

    return true;
}
