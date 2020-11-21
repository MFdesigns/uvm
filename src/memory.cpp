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

#include "memory_manip.hpp"
#include <iostream>

/**
 * Constructs a new MemBuffer
 * @param startAddr Virtual start address of memory buffer
 * @param size Size in bytes of memory section
 */
MemBuffer::MemBuffer(uint64_t startAddr, uint32_t size) : VStartAddr(startAddr), Size(size) {
    PhysicalBuffer = std::make_unique<uint8_t[]>(size);
}

/**
 * Move constructor of MemBuffer
 * @param memBuffer MemBuffer to be moved
 */
MemBuffer::MemBuffer(MemBuffer&& memBuffer) noexcept
    : VStartAddr(memBuffer.VStartAddr), Size(memBuffer.Size),
      PhysicalBuffer(std::move(memBuffer.PhysicalBuffer)) {}

/**
 * Get the underlying buffer
 * @return Pointer to buffer
 */
uint8_t* MemBuffer::getBuffer() const {
    return PhysicalBuffer.get();
}

/**
 * Constructs a new MemSection
 * @param type Section type
 * @param Perm Section permissions
 * @param startAddr Section start address
 * @param size Section size
 * @param buffIndex Index into buffer vector
 */
MemSection::MemSection(SectionType type, uint8_t perm, uint64_t startAddr, uint32_t size, uint32_t buffIndex): Type(type), Perm(perm), VStartAddr(startAddr), Size(size), BufferIndex(buffIndex) {}


/**
 * Finds a section which contains the given memory range
 * @param vAddr Memory range start address
 * @param size Memory range size in bytes
 * @return On success returns pointer to MemSection otherwise nullptr
 */
MemSection* MemManager::findSection(uint64_t vAddr, uint32_t size) const {
    MemSection* sec = nullptr;
    for (const MemSection& memSec: Sections) {
        if (vAddr >= memSec.VStartAddr && vAddr + size <= memSec.VStartAddr + memSec.Size) {
            sec = const_cast<MemSection*>(&memSec);
        }
    }
    return sec;
}

bool MemManager::readPhysicalMem(uint64_t vAddr,
                 uint32_t size,
                 uint8_t perms,
                 uint8_t** ptr) const {
    MemSection* section = findSection(vAddr, size);
    if (section == nullptr) {
        std::cout << "[Error] Failed reading memory at address 0x" << std::hex << vAddr << "\n\tmemory not owned by programm";
        return false;
    }

    if ((section->Perm & perms) != perms) {
        std::cout << "[Error] Failed reading memory at address 0x" << std::hex << vAddr << "\n\tmissing read permission";
        return false;
    }

    // Get physical buffer address
    const MemBuffer* memBuffer = &Buffers[section->BufferIndex];
    uint64_t bufferOffset = vAddr - memBuffer->VStartAddr;
    uint8_t* buffer = reinterpret_cast<uint8_t*>(memBuffer->getBuffer());
    *ptr = &buffer[bufferOffset];

    return true;
}

bool MemManager::writePhysicalMem(void* source, uint64_t vAddr, uint32_t size) {
    uint8_t* dest = nullptr;
    if (!readPhysicalMem(vAddr, size, PERM_WRITE_MASK, &dest)) {
        std::cout << "[Error] Failed writing to memory at address 0x" << std::hex << vAddr << '\n';
        return false;
    }

    std::memcpy(dest, source, size);
    return true;
}

uint32_t MemManager::addBuffer(uint64_t vAddr, uint32_t size) {
    uint32_t buffIndex = Buffers.size();
    Buffers.emplace_back(vAddr, size);
    return buffIndex;
}
