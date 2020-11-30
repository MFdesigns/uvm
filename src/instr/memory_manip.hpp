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

#pragma once
#include "../uvm.hpp"

namespace Instr {
// Push
bool pushInt(UVM* vm, uint32_t width, IntType type);
bool pushIReg(UVM* vm);
// Pop
bool pop(UVM* vm);
bool popIReg(UVM* vm);
// Load
bool loadIntToIReg(UVM* vm, uint32_t width, IntType type);
bool loadROToIReg(UVM* vm, uint32_t width);
// Store
bool storeIRegToRO(UVM* vm);
// Copy
bool copyIntToRO(UVM* vm, uint32_t width, IntType type);
bool copyIRegToIReg(UVM* vm);
bool copyROToRO(UVM* vm);
// Lea
bool leaROToIReg(UVM* vm);
} // namespace Instr
