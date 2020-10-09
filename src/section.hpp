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

enum class SectionType {
    NAME_STRING = 0x1,
    META_DATA = 0x2,
    DEBUG = 0x3,
    STATIC = 0x4,
    CODE = 0x5,
    STACK = 0x6,
};

constexpr uint8_t PERM_READ_MASK = 0b1000'0000;
constexpr uint8_t PERM_WRITE_MASK = 0b0100'0000;
constexpr uint8_t PERM_EXE_MASK = 0b0010'0000;

bool comparePerms(uint8_t permA, uint8_t permB);

class MemBuffer {
  public:
    MemBuffer(uint64_t vStartAddress, uint64_t size, uint8_t* buffer);
    MemBuffer(MemBuffer&& memBuffer) noexcept;
    uint8_t* getBuffer() const;
    uint64_t getSize() const;
    uint64_t getStartAddr() const;

  private:
    uint64_t VStartAddress = 0;
    uint64_t Size = 0;
    std::unique_ptr<uint8_t[]> Buffer;
};

struct MemSection {
    MemSection(uint64_t startAddress,
               uint64_t size,
               uint64_t nameAddress,
               SectionType type,
               uint8_t perms,
               uint32_t buffer);
    uint64_t VStartAddress = 0;
    uint64_t Size = 0;
    uint64_t VNameAddress = 0;
    SectionType Type;
    uint8_t Perms;
    uint32_t BufferIndex = 0;
};
