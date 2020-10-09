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
#include <cstdint>
#include <memory>
#include <vector>

constexpr uint8_t REG_INSTR_PTR = 0x1;
constexpr uint8_t REG_STACK_PTR = 0x2;
constexpr uint8_t REG_BASE_PTR = 0x3;
constexpr uint8_t REG_FLAGS = 0x4;
constexpr uint8_t REG_GP_START = 0x5;
constexpr uint8_t REG_GP_END = 0x15;
constexpr uint8_t REG_FP_START = 0x16;
constexpr uint8_t REG_FP_END = 0x26;

enum class IntType {
    I8 = 0x1,
    I16 = 0x2,
    I32 = 0x3,
    I64 = 0x4,
};

enum class FloatType {
    F32 = 0xF0,
    F64 = 0xF1,
};

union IntVal {
    uint8_t I8;
    uint16_t I16;
    uint32_t I32;
    uint64_t I64 = 0;
};

union FloatVal {
    float F32;
    double F64;
};

struct UVMInt {
    UVMInt(IntType type, IntVal val);
    IntType Type;
    IntVal Val;
};

struct UVMFloat {
    UVMFloat(FloatType type, FloatVal val);
    FloatType Type;
    FloatVal Val;
};

bool parseIntType(uint8_t type, IntType* intType);
bool parseFloatType(uint8_t type, FloatType* floatType);

class RegisterManager {
  public:
    RegisterManager();
    bool setIntReg(uint8_t id, UVMInt* val);
    bool getIntReg(uint8_t id, IntType type, IntVal* outVal) const;
    bool setFloatReg(uint8_t id, UVMFloat* val);
    bool getFloatReg(uint8_t id, FloatType type, FloatVal* outVal) const;

    // Internal functions can only be used by the VM internally and are not
    // allowed to be called by Bytecode directly
    uint64_t internalGetIP() const;
    void internalSetIP(uint64_t val);
    void internalSetSP(uint64_t val);
    void internalSetBP(uint64_t val);

  private:
    bool validateIntRegAccess(uint8_t id, IntVal** targetReg) const;
    bool validateFloatRegAccess(uint8_t id, FloatVal** targetReg) const;
    IntVal IP;
    IntVal SP;
    IntVal BP;
    IntVal FL;
    std::vector<IntVal> GP;
    std::vector<FloatVal> FP;
};
