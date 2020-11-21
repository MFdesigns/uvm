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
#include "memory.hpp"
#include "register.hpp"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

constexpr uint64_t UVM_START_ADDR = 0;
constexpr uint64_t UVM_STACK_SIZE = 4096;

// Instruction opcodes
constexpr uint8_t OP_STORE_IT_IR_RO = 0x08;
constexpr uint8_t OP_LEA_RO_IR = 0x10;
constexpr uint8_t OP_LOAD_I8_IR = 0x11;
constexpr uint8_t OP_LOAD_I16_IR = 0x12;
constexpr uint8_t OP_LOAD_I32_IR = 0x13;
constexpr uint8_t OP_LOAD_I64_IR = 0x14;
constexpr uint8_t OP_LOAD_IT_RO_IR = 0x15;
constexpr uint8_t OP_COPY_I8_RO = 0x21;
constexpr uint8_t OP_COPY_I16_RO = 0x22;
constexpr uint8_t OP_COPY_I32_RO = 0x23;
constexpr uint8_t OP_COPY_I64_RO = 0x24;
constexpr uint8_t OP_COPY_IT_IR_IR = 0x25;
constexpr uint8_t OP_COPY_IT_RO_RO = 0x26;
constexpr uint8_t OP_SYS = 0x40;
constexpr uint8_t OP_EXIT = 0x50;

struct HeaderInfo {
    uint8_t Version = 0;
    uint8_t Mode = 0;
    uint64_t StartAddress = 0;
};

class UVM {
  public:
    UVM(std::filesystem::path p);
    bool init();
    bool run();
    RegisterManager RM;
    MemManager MMU;

  private:
    std::filesystem::path SourcePath;
    uint32_t SourceBuffIndex = 0;
    std::unique_ptr<HeaderInfo> HInfo;
    void readSource();
};
