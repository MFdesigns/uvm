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
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

constexpr uint64_t UVM_NULLPTR = 0;
constexpr uint64_t UVM_STACK_SIZE = 4096;
constexpr size_t HEAP_BLOCK_SIZE = 1024;
constexpr size_t MAX_INSTR_SIZE = 15;

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
    uint64_t I64;
    int8_t S8;
    int16_t S16;
    int32_t S32;
    int64_t S64 = 0;
};

union FloatVal {
    float F32;
    double F64 = 0.0f;
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

enum class MemType {
    NAME_STRING = 0x1,
    META_DATA = 0x2,
    DEBUG = 0x3,
    STATIC = 0x4,
    GLOBAL = 0x5,
    CODE = 0x6,
    STACK = 0x7,
    HEAP = 0x8,
};

struct MemBuffer {
    MemBuffer(uint64_t startAddr, uint32_t size, MemType type, uint8_t perm);
    MemBuffer(MemBuffer&& memBuffer) noexcept;
    MemBuffer& operator=(MemBuffer&& memBuffer) noexcept;
    ~MemBuffer();
    /** Virtual start address of physical buffer */
    uint64_t VStartAddr = 0;
    /** Size of buffer in bytes */
    uint32_t Size = 0;
    /** Type of the section */
    MemType Type;
    /** Section permissions */
    uint8_t Perm = 0;
    /** Heap block capacity **/
    uint32_t Capacity = 0;
    /** How much has been freed **/
    uint32_t Freed = 0;

    void read(void* source);

    /** Physical buffer */
    uint8_t* Buffer = nullptr;
};

struct MemSection {
    MemSection(MemType type, uint8_t perm, uint64_t startAddr, uint32_t size);
    const MemType Type;
    /** Section permissions */
    const uint8_t Perm = 0;
    /** Virtual start address of section */
    const uint64_t VStartAddr = 0;
    /** Size of section in bytes */
    const uint32_t Size = 0;
};

struct FlagsRegister {
    bool Carry = false;
    bool Zero = false;
    bool Signed = false;
};

struct MemManager {
    /** list of sections */
    std::vector<MemSection> Sections;
    /** list of memory buffers */
    std::vector<MemBuffer> Buffers;
    /** index to stack buffer inside buffers array */
    uint32_t StackBufferIndex = 0;
    /** virtual address of stack start */
    uint64_t VStackStart = 0;
    /** virtual address of stack end */
    uint64_t VStackEnd = 0;
    /** Pointer to top of heap */
    uint64_t VHeapStart = 0;
    /** Instruction pointer */
    uint64_t IP = 0;
    /** Stack pointer */
    uint64_t SP = 0;
    /** Base pointer */
    uint64_t BP = 0;
    /** Flags register */
    FlagsRegister Flags;
    /** General purpose registers r0 - r15 */
    std::array<IntVal, 16> GP = {0};
    /** Floating point registers f0 - f15 */
    std::array<FloatVal, 16> FP = {0};
    /** Current instruction buffer */
    std::array<uint8_t, MAX_INSTR_SIZE> InstrBuffer;

    MemSection* findSection(uint64_t vAddr, uint32_t size) const;
    uint32_t read(uint64_t vAddr, void* dest, UVMDataSize size, uint8_t perm);
    uint32_t write(void* src, uint64_t vAddr, UVMDataSize size, uint8_t perm);
    uint32_t readLarge(uint64_t vAddr, void* dest, uint32_t size, uint8_t perm);
    uint32_t writeLarge(void* src, uint64_t vAddr, uint32_t size, uint8_t perm);
    uint32_t fetchInstruction(uint8_t* dest, size_t size);
    uint32_t
    addBuffer(uint64_t vAddr, uint32_t size, MemType type, uint8_t perm);
    void initStack();
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
    uint32_t deallocHeap(uint64_t vAddr);
    void loadSections(uint8_t* buff, size_t size);
};

bool parseIntType(uint8_t type, IntType* intType);
bool parseFloatType(uint8_t type, FloatType* floatType);
