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

#pragma once
#include <cstdint>
#include <iostream>

// clang-format off
// Successful values
constexpr uint32_t UVM_SUCCESS = 0;
constexpr uint32_t UVM_SUCCESS_JUMPED = 1;

// File errors
constexpr uint32_t E_INVALID_HEADER =       0xFE000;
constexpr uint32_t E_INVALID_START_ADDR =   0xFE001;
constexpr uint32_t E_INVALID_SEC_TABLE =    0xFE002;

// Runtime errors
constexpr uint32_t E_UNKNOWN_OP_CODE =          0xE000;
constexpr uint32_t E_VADDR_NOT_FOUND =          0xE001;
constexpr uint32_t E_DEALLOC_INVALID_ADDR =     0xE002;
constexpr uint32_t E_INVALID_JUMP_DEST =        0xE003;
constexpr uint32_t E_MISSING_PERM =             0xE004;
constexpr uint32_t E_INVALID_READ =             0xE005;
constexpr uint32_t E_INVALID_WRITE =            0xE006;
constexpr uint32_t E_INVALID_TYPE =             0xE007;
constexpr uint32_t E_INVALID_SRC_REG =          0xE008;
constexpr uint32_t E_INVALID_DEST_REG =         0xE009;
constexpr uint32_t E_INVALID_SRC_REG_OFFSET =   0xE00A;
constexpr uint32_t E_INVALID_DEST_REG_OFFSET =  0xE00B;
constexpr uint32_t E_SYSCALL_UNKNOWN =          0xE00C;
constexpr uint32_t E_SYSCALL_FAILURE =          0xE00D;
constexpr uint32_t E_DIVISON_ZERO =             0xE00E;
constexpr uint32_t E_INVALID_STACK_OP =         0xE00F;
constexpr uint32_t E_INVALID_BASE_PTR =         0xE010;
// clang-format on

const char* translateError(uint8_t errCode);
