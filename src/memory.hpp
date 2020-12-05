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
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

constexpr uint64_t UVM_STACK_SIZE = 4096;
constexpr size_t HEAP_BLOCK_SIZE = 1024;
constexpr uint64_t UVM_NULLPTR = 0;

// Stack error codes
constexpr uint32_t ERR_STACK_OVERFLOW = 0xE0;
constexpr uint32_t ERR_STACK_UNDERFLOW = 0xE1;
constexpr uint32_t ERR_BASE_PTR_OUTSIDE_STACK = 0xE1;

// Register error codes
constexpr uint32_t ERR_REG_NO_PERM = 0xE2;
constexpr uint32_t ERR_REG_UNKNOWN_ID = 0xE3;
constexpr uint32_t ERR_REG_TYPE_MISMATCH = 0xE4;

constexpr uint8_t PERM_READ_MASK = 0b1000'0000;
constexpr uint8_t PERM_WRITE_MASK = 0b0100'0000;
constexpr uint8_t PERM_EXE_MASK = 0b0010'0000;

constexpr uint8_t REG_INSTR_PTR = 0x1;
constexpr uint8_t REG_STACK_PTR = 0x2;
constexpr uint8_t REG_BASE_PTR = 0x3;
constexpr uint8_t REG_FLAGS = 0x4;
constexpr uint8_t REG_GP_START = 0x5;
constexpr uint8_t REG_GP_END = 0x15;
constexpr uint8_t REG_FP_START = 0x16;
constexpr uint8_t REG_FP_END = 0x26;

enum class UVMDataSize {
    BYTE = 1,  // i8
    WORD = 2,  // i16
    DWORD = 4, // i32 / f32
    QWORD = 8, // i64 / f64
};

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

enum class SectionType {
    NAME_STRING = 0x1,
    META_DATA = 0x2,
    DEBUG = 0x3,
    STATIC = 0x4,
    CODE = 0x5,
    STACK = 0x6,
};

class MemBuffer {
  public:
    MemBuffer(uint64_t startAddr, uint32_t size);
    MemBuffer(MemBuffer&& memBuffer) noexcept;
    /** Virtual start address of physical buffer */
    const uint64_t VStartAddr = 0;
    /** Size of buffer in bytes */
    const uint32_t Size = 0;
    uint8_t* getBuffer() const;

  private:
    /** Physical buffer */
    std::unique_ptr<uint8_t[]> PhysicalBuffer;
};

// TODO: What about section name strings?
class MemSection {
  public:
    MemSection(SectionType type,
               uint8_t perm,
               uint64_t startAddr,
               uint32_t size,
               uint32_t buffIndex);
    const SectionType Type;
    /** Section permissions */
    const uint8_t Perm = 0;
    /** Virtual start address of section */
    const uint64_t VStartAddr = 0;
    /** Size of section in bytes */
    const uint32_t Size = 0;
    /** Index to physical buffer in MemBuffer vector */
    const uint32_t BufferIndex = 0;
};

class HeapBlock {
  public:
    HeapBlock(size_t size, uint64_t start);
    HeapBlock(HeapBlock&& heapBlock);
    size_t Size = 0;
    uint64_t VStart = 0;
    size_t Capacity = 0;
    size_t Freed = 0;
    std::unique_ptr<uint8_t[]> Buffer;
};

bool parseIntType(uint8_t type, IntType* intType);
bool parseFloatType(uint8_t type, FloatType* floatType);

class MemManager {
  public:
    std::vector<MemSection> Sections;
    std::vector<MemBuffer> Buffers;
    std::vector<HeapBlock> Heap;

    uint32_t StackBufferIndex = 0;
    uint64_t VStackStart = 0;
    uint64_t VStackEnd = 0;

    uint64_t IP = 0;
    uint64_t SP = 0;
    uint64_t BP = 0;
    std::array<IntVal, 16> GP = {0};
    std::array<FloatVal, 16> FP = {0};

    MemSection* findSection(uint64_t vAddr, uint32_t size) const;
    bool readPhysicalMem(uint64_t vAddr,
                         uint32_t size,
                         uint8_t perms,
                         uint8_t** ptr) const;
    bool writePhysicalMem(void* source, uint64_t vAddr, uint32_t size);
    uint32_t addBuffer(uint64_t vAddr, uint32_t size);
    void initStack(uint64_t vAddr);

    uint32_t setStackPtr(uint64_t vAddr);
    uint32_t setBasePtr(uint64_t vAddr);
    uint32_t stackPush(void* val, UVMDataSize size);
    uint32_t stackPop(uint64_t* out, UVMDataSize size);

    uint32_t setIntReg(uint8_t id, IntVal val, IntType type);
    uint32_t setFloatReg(uint8_t id, FloatVal val, FloatType type);

    uint32_t getIntReg(uint8_t id, IntVal& val);
    uint32_t getFloatReg(uint8_t id, FloatVal& val);

    bool evalRegOffset(uint8_t* buff, uint64_t* address);

    uint64_t allocHeap(size_t size);
    bool deallocHeap(uint64_t vAddr);

  private:
    uint64_t HeapPointer = 0;
};
