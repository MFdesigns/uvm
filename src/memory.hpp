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
#include <cstdint>
#include <vector>
#include <memory>

constexpr uint8_t PERM_READ_MASK = 0b1000'0000;
constexpr uint8_t PERM_WRITE_MASK = 0b0100'0000;
constexpr uint8_t PERM_EXE_MASK = 0b0010'0000;

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
    MemSection(SectionType type, uint8_t perm, uint64_t startAddr, uint32_t size, uint32_t buffIndex);
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

class MemManager {
public:
    MemSection* findSection(uint64_t vAddr, uint32_t size) const;
    bool readPhysicalMem(uint64_t vAddr,
                 uint32_t size,
                 uint8_t perms,
                 uint8_t** ptr) const;
    bool writePhysicalMem(void* source, uint64_t vAddr, uint32_t size);
    uint32_t addBuffer(uint64_t vAddr, uint32_t size);
    uint8_t* getBuffer(uint32_t index);

    std::vector<MemSection> Sections;
    std::vector<MemBuffer> Buffers;
};
