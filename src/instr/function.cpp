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

#include "function.hpp"
#include <iostream>

bool Instr::call(UVM* vm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, 9, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    // Get target vAddr
    uint64_t* targetAddr = reinterpret_cast<uint64_t*>(&buff[1]);

    uint64_t currentIP = vm->MMU.IP;
    if (vm->MMU.stackPush(&currentIP, UVMDataSize::QWORD) != 0) {
        return false;
    }

    MemSection* memSec = vm->MMU.findSection(*targetAddr, 1);
    if (memSec == nullptr) {
        std::cout << "[Error] Call to unknown memory address 0x" << std::hex
                  << *targetAddr << '\n';
        return false;
    }

    if ((memSec->Perm & PERM_EXE_MASK) != PERM_EXE_MASK) {
        std::cout << "[Error] Call to section with missing executable "
                     "permisson at address 0x"
                  << std::hex << *targetAddr << '\n';
        return false;
    }

    vm->MMU.IP = *targetAddr;

    return true;
}

bool Instr::ret(UVM* vm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, 1, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint64_t targetIP = 0;
    if (vm->MMU.stackPop(&targetIP, UVMDataSize::QWORD) != 0) {
        return false;
    }

    MemSection* memSec = vm->MMU.findSection(targetIP, 1);
    if (memSec == nullptr) {
        std::cout << "[Error] Ret instruction to unknown memory address 0x"
                  << std::hex << targetIP << '\n';
        return false;
    }

    if ((memSec->Perm & PERM_EXE_MASK) != PERM_EXE_MASK) {
        std::cout << "[Error] Ret instruction to section with missing "
                     "executable permisson at address 0x"
                  << std::hex << targetIP << '\n';
        return false;
    }

    vm->MMU.IP = targetIP;

    return true;
}
