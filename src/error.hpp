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
constexpr uint32_t UVM_SUCCESS_JUMPED = 5;

// Error values
constexpr uint32_t E_INVALID_HEADER = 1;
constexpr uint32_t E_INVALID_SEC_TABLE = 2;
constexpr uint32_t E_VADDR_NOT_FOUND = 3;
constexpr uint32_t E_MISSING_PERM = 4;
