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

#pragma once
#include "../uvm.hpp"

namespace Instr {
// Push
uint32_t pushInt(UVM* vm, uint32_t width, uint32_t flag);
uint32_t pushIReg(UVM* vm, uint32_t width, uint32_t flag);
uint32_t pop(UVM* vm, uint32_t width, uint32_t flag);
uint32_t popIReg(UVM* vm, uint32_t width, uint32_t flag);
uint32_t loadIntToIReg(UVM* vm, uint32_t width, uint32_t flag);
uint32_t loadROToIReg(UVM* vm, uint32_t width, uint32_t flag);
uint32_t storeIRegToRO(UVM* vm, uint32_t width, uint32_t flag);
uint32_t copyIntToRO(UVM* vm, uint32_t width, uint32_t flag);
uint32_t copyIRegToIReg(UVM* vm, uint32_t width, uint32_t flag);
uint32_t copyROToRO(UVM* vm, uint32_t width, uint32_t flag);
uint32_t leaROToIReg(UVM* vm, uint32_t width, uint32_t flag);
} // namespace Instr
