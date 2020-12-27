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
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>
#include <sstream>

constexpr uint64_t UVM_START_ADDR = 0;

// Instruction opcodes
constexpr uint8_t OP_PUSH_I8 = 0x01;
constexpr uint8_t OP_PUSH_I16 = 0x02;
constexpr uint8_t OP_PUSH_I32 = 0x03;
constexpr uint8_t OP_PUSH_I64 = 0x04;
constexpr uint8_t OP_PUSH_IT_IR = 0x05;
constexpr uint8_t OP_POP_IT = 0x06;
constexpr uint8_t OP_POP_IT_IR = 0x07;
constexpr uint8_t OP_STORE_IT_IR_RO = 0x08;
constexpr uint8_t OP_LEA_RO_IR = 0x10;
constexpr uint8_t OP_LOAD_I8_IR = 0x11;
constexpr uint8_t OP_LOAD_I16_IR = 0x12;
constexpr uint8_t OP_LOAD_I32_IR = 0x13;
constexpr uint8_t OP_LOAD_I64_IR = 0x14;
constexpr uint8_t OP_LOAD_IT_RO_IR = 0x15;
constexpr uint8_t OP_CALL = 0x20;
constexpr uint8_t OP_COPY_I8_RO = 0x21;
constexpr uint8_t OP_COPY_I16_RO = 0x22;
constexpr uint8_t OP_COPY_I32_RO = 0x23;
constexpr uint8_t OP_COPY_I64_RO = 0x24;
constexpr uint8_t OP_COPY_IT_IR_IR = 0x25;
constexpr uint8_t OP_COPY_IT_RO_RO = 0x26;
constexpr uint8_t OP_RET = 0x30;
constexpr uint8_t OP_ADD_IT_IR_IR = 0x35;
constexpr uint8_t OP_SUB_IT_IR_IR = 0x45;
constexpr uint8_t OP_MUL_IT_IR_IR = 0x55;
constexpr uint8_t OP_DIV_IT_IR_IR = 0x65;
constexpr uint8_t OP_SYS = 0x40;
constexpr uint8_t OP_EXIT = 0x50;
constexpr uint8_t OP_NOP = 0xA0;
constexpr uint8_t OP_B2L = 0xB1;
constexpr uint8_t OP_S2L = 0xB2;
constexpr uint8_t OP_I2L = 0xB3;
constexpr uint8_t OP_CMP_IT_IR_IR = 0xD1;
constexpr uint8_t OP_JMP = 0xE1;
constexpr uint8_t OP_JE = 0xE2;
constexpr uint8_t OP_JNE = 0xE3;
constexpr uint8_t OP_JGT = 0xE4;
constexpr uint8_t OP_JLT = 0xE5;
constexpr uint8_t OP_JGE = 0xE6;
constexpr uint8_t OP_JLE = 0xE7;

struct HeaderInfo {
    uint8_t Version = 0;
    uint8_t Mode = 0;
    uint64_t StartAddress = 0;
};

enum class ExecutionMode {
    USER,
    DEBUGGER,
};

class UVM {
  public:
    MemManager MMU;
    uint8_t Opcode = 0;
    ExecutionMode Mode = ExecutionMode::USER;
    std::stringstream DbgConsole;
    void setFilePath(std::filesystem::path p);
    bool init();
    bool run();
    bool nextInstr();
    void addSourceFromBuffer(uint8_t* srcBuffer, size_t size);
    void readSource();

  private:
    std::filesystem::path SourcePath;
    uint32_t SourceBuffIndex = 0;
    HeaderInfo HInfo;
};
