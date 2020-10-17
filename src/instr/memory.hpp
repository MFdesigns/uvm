/**
 * Copyright 2020 Michel Fäh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include "../register.hpp"
#include "../uvm.hpp"

namespace Instr {
bool copyIntToRO(UVM* vm, RegisterManager* rm, uint32_t width, IntType type);
bool copyIRegToIReg(UVM* vm, RegisterManager* rm);
bool loadIntToIReg(UVM* vm, RegisterManager* rm, uint32_t width, IntType type);
bool loadROToIReg(UVM* vm, RegisterManager* rm, uint32_t width);
} // namespace Instr
