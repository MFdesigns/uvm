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
#include <cstdio>
#include <ctime>
#include <memory>

/**
 * Performs syscall for printing to console
 * @param vm UVM instance
 * @return On success returns true otherwise false
 */
bool syscall_print(UVM* vm) {
    // Arguments:
    // r0: uint64_t string pointer
    // r1: uint32_t string size

    // Return values:
    // r0: uint64_t ptr to allocated mem block

    IntVal r0 = vm->MMU.GP[0];
    IntVal r1 = vm->MMU.GP[1];

    uint32_t stringSize = r1.I32;

    // Create temporary string buffer (is not \0 terminated)
    std::unique_ptr<char[]> buff = std::make_unique<char[]>(stringSize);

    uint32_t readRes = vm->MMU.readLarge(r0.I64, buff.get(), stringSize, 0);
    if (readRes != UVM_SUCCESS) {
        return false;
    }

    // Depending from what context the VM was started the output will either go
    // to stdout or into a console buffer which will later be sent to the debug
    // client
    switch (vm->Mode) {
    case ExecutionMode::USER:
        fwrite(buff.get(), 1, stringSize, stdout);
        break;
    case ExecutionMode::DEBUGGER:
        vm->DbgConsole.write(buff.get(), stringSize);
        break;
    }

    return true;
}

/**
 * Performs syscall for reading from console
 * @param vm UVM instance
 * @return On success returns true otherwise false
 */
bool syscall_console_read(UVM* vm) {
    // Arguments:
    // r0: double ptr to heap allocated string
    // r1: ptr to uint32_t string size

    // Return values:
    // -

    IntVal strPtrPtr = vm->MMU.GP[0];
    IntVal strSizePtr = vm->MMU.GP[1];

    std::string str;
    std::getline(std::cin, str);
    uint32_t strSize = str.size();

    uint64_t strPtr = vm->MMU.allocHeap(strSize);

    // Copy string into vm heap
    bool writeStatus =
        vm->MMU.writeLarge(const_cast<char*>(str.c_str()), strPtr, strSize, 0);
    if (writeStatus != UVM_SUCCESS) {
        return false;
    }

    // Write out vaddr of heap string
    bool writeStrPtrStatus = vm->MMU.write(&strPtr, strPtrPtr.I64,
                                           UVMDataSize::QWORD, PERM_WRITE_MASK);
    if (writeStrPtrStatus != UVM_SUCCESS) {
        return false;
    }

    // Write out string size
    bool writeStrSizeStatus = vm->MMU.write(
        &strSize, strSizePtr.I64, UVMDataSize::DWORD, PERM_WRITE_MASK);
    if (writeStrSizeStatus != UVM_SUCCESS) {
        return false;
    }

    return true;
}

/**
 * Performs syscall for memory allocation
 * @param vm UVM instance
 */
void syscall_alloc(UVM* vm) {
    // Arguments:
    // r0: uint32_t alloc size

    // Return values:
    // r0: uint64_t ptr to allocated mem block

    IntVal allocSize = vm->MMU.GP[0];

    IntVal allocAddr;
    allocAddr.I64 = vm->MMU.allocHeap(allocSize.I32);

    vm->MMU.GP[0] = allocAddr;
}

/**
 * Performs syscall for deallocating previously allocated memory
 * @param vm UVM instance
 * @return On success returns true otherwise false
 */
bool syscall_dealloc(UVM* vm) {
    // Arguments:
    // r0: uint64_t heap address

    // Return values:
    // -

    IntVal vAddr = vm->MMU.GP[0];

    uint32_t deallocRes = vm->MMU.deallocHeap(vAddr.I64);
    if (deallocRes != UVM_SUCCESS) {
        return false;
    }

    return true;
}

/**
 * Performs syscall for geting the current time
 * @param vm UVM instance
 * @return On success returns true otherwise false
 */
bool syscall_time(UVM* vm) {
    // Arguments:
    // -

    // Return values:
    // r0: uint64_t POSIX time

    std::time_t currentTime = time(nullptr);
    if (currentTime == (std::time_t)(-1)) {
        return false;
    }

    vm->MMU.GP[0].I64 = static_cast<uint64_t>(currentTime);

    return true;
}

/**
 * Selects correct syscall and executes it
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_SYSCALL_UNKNOWN, E_SYSCALL_FAILURE]
 */
uint32_t instr_syscall(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // sys <sysID>

    constexpr uint32_t SYS_TYPE_OFFSET = 1;

    uint8_t syscallType = vm->MMU.InstrBuffer[SYS_TYPE_OFFSET];
    bool callSuccess = true;
    switch (syscallType) {
    case SYSCALL_PRINT:
        callSuccess = syscall_print(vm);
        break;
    case SYSCALL_CONSOLE_READ:
        callSuccess = syscall_console_read(vm);
        break;
    case SYSCALL_ALLOC:
        syscall_alloc(vm);
        break;
    case SYSCALL_DEALLOC:
        callSuccess = syscall_dealloc(vm);
        break;
    case SYSCALL_TIME: {
        callSuccess = syscall_time(vm);
    } break;
    default:
        return E_SYSCALL_UNKNOWN;
    }

    if (!callSuccess) {
        return E_SYSCALL_FAILURE;
    }

    return UVM_SUCCESS;
}
