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
#include "../uvm.hpp"
#include <cstdint>

enum class JumpCondition {
    UNCONDITIONAL,
    IF_EQUALS,
    IF_NOT_EQUALS,
    IF_GREATER_THAN,
    IF_LESS_THAN,
    IF_GREATER_EQUALS,
    IF_LESS_EQUALS,
};

// TODO: Move into UVM
bool internalPrint(UVM* vm);

namespace Instr {
// Arithmetic
uint32_t addIRegToIReg(UVM* vm, uint32_t width, uint32_t flag);
uint32_t subIRegFromIReg(UVM* vm, uint32_t width, uint32_t flag);
uint32_t mulIRegWithIReg(UVM* vm, uint32_t width, uint32_t flag);
uint32_t divIRegByIReg(UVM* vm, uint32_t width, uint32_t flag);
uint32_t uIntConvert(UVM* vm, uint32_t width, uint32_t flag);
// Branching
uint32_t cmpIRegToIReg(UVM* vm, uint32_t width, uint32_t flag);
uint32_t jmp(UVM* vm, uint32_t width, uint32_t flag);
// Function
uint32_t call(UVM* vm, uint32_t width, uint32_t flag);
uint32_t ret(UVM* vm, uint32_t width, uint32_t flag);
// Memory manip
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
// Syscall
uint32_t syscall(UVM* vm, uint32_t width, uint32_t flag);
} // namespace Instr
