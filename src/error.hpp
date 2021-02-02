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

// Successful values
constexpr uint32_t UVM_SUCCESS = 0;
constexpr uint32_t UVM_SUCCESS_JUMPED = 1;

// TODO: Replace E_INVALID_REG_OFFSET with E_INVALID_SOURCE_REG_OFFSET or E_INVALID_DEST_REG_OFFSET

// Error values
constexpr uint32_t E_INVALID_HEADER = 20;
constexpr uint32_t E_INVALID_SEC_TABLE = 21;
constexpr uint32_t E_VADDR_NOT_FOUND = 22;
constexpr uint32_t E_MISSING_PERM = 23;
constexpr uint32_t E_DEALLOC_INVALID_ADDR = 24;
constexpr uint32_t E_INVALID_TARGET_REG = 25;
constexpr uint32_t E_INVALID_TYPE = 26;
constexpr uint32_t E_INVALID_REG_OFFSET = 27;
constexpr uint32_t E_INVALID_READ = 28;
constexpr uint32_t E_INVALID_SOURCE_REG = 29;
constexpr uint32_t E_INVALID_WRITE = 30;
constexpr uint32_t E_INVALID_SOURCE_REG_OFFSET = 31;
constexpr uint32_t E_INVALID_DEST_REG_OFFSET = 32;
