// ======================================================================== //
// Copyright 2021 Michel Fäh
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

#include "error.hpp"

/**
 * Translate error code into error message
 * @param errCode Error to translate
 */
const char* translateRuntimeError(uint8_t errCode) {
    char* strPtr = nullptr;
    switch (errCode) {
    case E_INVALID_HEADER:
        strPtr = "invalid header";
        break;
    case E_INVALID_SEC_TABLE:
        strPtr = "invalid section table";
        break;
    case E_VADDR_NOT_FOUND:
        strPtr = "virtual address not found";
        break;
    case E_MISSING_PERM:
        strPtr = "target memory missing required permission";
        break;
    case E_DEALLOC_INVALID_ADDR:
        strPtr = "provided invalid address to deallocation";
        break;
    case E_INVALID_TARGET_REG:
        strPtr = "invalid target register";
        break;
    case E_INVALID_REG_OFFSET:
        strPtr = "invalid register offset";
        break;
    case E_INVALID_READ:
        strPtr = "invalid read from given address";
        break;
    case E_INVALID_SOURCE_REG:
        strPtr = "invalid source register";
        break;
    case E_INVALID_WRITE:
        strPtr = "invalid write to given address";
        break;
    case E_INVALID_SOURCE_REG_OFFSET:
        strPtr = "invalid source register offset";
        break;
    case E_INVALID_DEST_REG_OFFSET:
        strPtr = "invalid destination register offset";
        break;
    case E_INVALID_STACK_OPERATION:
        strPtr = "invalid stack operation";
        break;
    case E_INVALID_JUMP_DEST:
        strPtr = "invalid jump destination address";
        break;
    case E_SYSCALL_UNKNOWN:
        strPtr = "unknown system call";
        break;
    case E_SYSCALL_FAILURE:
        strPtr = "could not perform system call";
        break;
    case E_DIVISON_ZERO:
        strPtr = "divison by zero";
        break;
    case E_UNKNOWN_OP_CODE:
        strPtr = "unknown opcode";
        break;
    }

    return const_cast<const char*>(strPtr);
}
