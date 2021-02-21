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
 * Call instruction pushes the ip on top off the stack and performs an
 * unconditional jump the target virtual address.
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS_JUMPED otherwise error state
 * [E_INVALID_STACK_OP, E_INVALID_JUMP_DEST, E_MISSING_PERM]
 */
uint32_t instr_call(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // call <vAddr>

    constexpr uint32_t ADDR_OFFSET = 1;

    // Get target vAddr
    uint64_t* targetAddr =
        reinterpret_cast<uint64_t*>(&vm->MMU.InstrBuffer[ADDR_OFFSET]);

    // Push next instruction pointer
    uint64_t currentIP = vm->MMU.IP + width;
    if (vm->MMU.stackPush(&currentIP, UVMDataSize::QWORD) != 0) {
        return E_INVALID_STACK_OP;
    }

    MemSection* memSec = vm->MMU.findSection(*targetAddr, 1);
    if (memSec == nullptr) {
        return E_INVALID_JUMP_DEST;
    }

    if ((memSec->Perm & PERM_EXE_MASK) != PERM_EXE_MASK) {
        return E_MISSING_PERM;
    }

    vm->MMU.IP = *targetAddr;

    return UVM_SUCCESS_JUMPED;
}

/**
 * Return instruction pops the virtual address on top of the stack into the
 * instruction pointer and thus return to the caller.
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS_JUMPED otherwise error state
 * [E_INVALID_STACK_OP, E_INVALID_JUMP_DEST, E_MISSING_PERM]
 */
uint32_t instr_ret(UVM* vm, uint32_t width, uint32_t flag) {
    uint64_t targetIP = 0;
    if (vm->MMU.stackPop(&targetIP, UVMDataSize::QWORD) != 0) {
        return E_INVALID_STACK_OP;
    }

    MemSection* memSec = vm->MMU.findSection(targetIP, 1);
    if (memSec == nullptr) {
        return E_INVALID_JUMP_DEST;
    }

    if ((memSec->Perm & PERM_EXE_MASK) != PERM_EXE_MASK) {
        return E_MISSING_PERM;
    }

    vm->MMU.IP = targetIP;

    return UVM_SUCCESS_JUMPED;
}
