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

uint32_t Instr::call(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t ADDR_OFFSET = 1;

    // Get target vAddr
    uint64_t* targetAddr =
        reinterpret_cast<uint64_t*>(&vm->MMU.InstrBuffer[ADDR_OFFSET]);

    // Push next instruction pointer
    uint64_t currentIP = vm->MMU.IP + 9;
    if (vm->MMU.stackPush(&currentIP, UVMDataSize::QWORD) != 0) {
        return 0xFF;
    }

    MemSection* memSec = vm->MMU.findSection(*targetAddr, 1);
    if (memSec == nullptr) {
        std::cout << "[Error] Call to unknown memory address 0x" << std::hex
                  << *targetAddr << '\n';
        return 0xFF;
    }

    if ((memSec->Perm & PERM_EXE_MASK) != PERM_EXE_MASK) {
        std::cout << "[Error] Call to section with missing executable "
                     "permisson at address 0x"
                  << std::hex << *targetAddr << '\n';
        return 0xFF;
    }

    vm->MMU.IP = *targetAddr;

    return UVM_SUCCESS_JUMPED;
}

uint32_t Instr::ret(UVM* vm, uint32_t width, uint32_t flag) {
    uint64_t targetIP = 0;
    if (vm->MMU.stackPop(&targetIP, UVMDataSize::QWORD) != 0) {
        return 0xFF;
    }

    MemSection* memSec = vm->MMU.findSection(targetIP, 1);
    if (memSec == nullptr) {
        std::cout << "[Error] Ret instruction to unknown memory address 0x"
                  << std::hex << targetIP << '\n';
        return 0xFF;
    }

    if ((memSec->Perm & PERM_EXE_MASK) != PERM_EXE_MASK) {
        std::cout << "[Error] Ret instruction to section with missing "
                     "executable permisson at address 0x"
                  << std::hex << targetIP << '\n';
        return 0xFF;
    }

    vm->MMU.IP = targetIP;

    return UVM_SUCCESS_JUMPED;
}
