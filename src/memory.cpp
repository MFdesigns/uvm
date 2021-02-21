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

#include "memory.hpp"
#include "error.hpp"
#include <cstring>
#include <iostream>
#include <vector>

/**
 * Constructs a new MemBuffer
 * @param startAddr Virtual start address of memory buffer
 * @param size Size in bytes of memory section
 * @param type Buffer type
 * @param perm Buffer permissions
 */
MemBuffer::MemBuffer(uint64_t startAddr,
                     uint32_t size,
                     MemType type,
                     uint8_t perm)
    : VStartAddr(startAddr), Size(size), Type(type), Perm(perm), Capacity(size),
      Buffer(new uint8_t[size]) {}

/** Move assignment operator */
MemBuffer& MemBuffer::operator=(MemBuffer&& memBuffer) noexcept {
    VStartAddr = memBuffer.VStartAddr;
    Size = memBuffer.Size;
    Type = memBuffer.Type;
    Perm = memBuffer.Perm;
    Capacity = memBuffer.Capacity;
    Freed = memBuffer.Freed;
    Buffer = memBuffer.Buffer;
    memBuffer.Buffer = nullptr;
    return *this;
}

/**
 * Move constructor of MemBuffer
 * @param memBuffer MemBuffer to be moved
 */
MemBuffer::MemBuffer(MemBuffer&& memBuffer) noexcept
    : VStartAddr(memBuffer.VStartAddr), Size(memBuffer.Size),
      Type(memBuffer.Type), Perm(memBuffer.Perm), Buffer(memBuffer.Buffer) {
    memBuffer.Buffer = nullptr;
}

/** Destructor */
MemBuffer::~MemBuffer() {
    delete[] Buffer;
}

/**
 * Copies memory of size given in member Size from source into member Buffer
 * @param source Pointer to source buffer
 */
void MemBuffer::read(void* source) {
    memcpy(Buffer, source, Size);
}

/**
 * Constructs a new MemSection
 * @param type Section type
 * @param Perm Section permissions
 * @param startAddr Section start address
 * @param size Section size
 * @param buffIndex Index into buffer vector
 */
MemSection::MemSection(MemType type,
                       uint8_t perm,
                       uint64_t startAddr,
                       uint32_t size)
    : Type(type), Perm(perm), VStartAddr(startAddr), Size(size) {}

/**
 * Finds a section which contains the given memory range
 * @param vAddr Memory range start address
 * @param size Memory range size in bytes
 * @return On success returns pointer to MemSection otherwise nullptr
 */
MemSection* MemManager::findSection(uint64_t vAddr, uint32_t size) const {
    MemSection* sec = nullptr;
    for (const MemSection& memSec : Sections) {
        if (vAddr >= memSec.VStartAddr &&
            vAddr + size <= memSec.VStartAddr + memSec.Size) {
            sec = const_cast<MemSection*>(&memSec);
        }
    }
    return sec;
}

/**
 * Sets the stack pointer to the new address if it passes the stack range check
 * @param vAddr New stack address
 * @return On success returns UVM_SUCCESS otherwise error code
 */
uint32_t MemManager::setStackPtr(uint64_t vAddr) {
    if (vAddr > VStackEnd || vAddr < VStackStart) {
        return E_INVALID_STACK_OP;
    }

    SP = vAddr;
    return UVM_SUCCESS;
}

/**
 * Sets the base pointer to the new address if it passes the stack range check
 * @param vAddr New base pointer address
 * @return On success returns UVM_SUCCESS otherwise error code
 */
uint32_t MemManager::setBasePtr(uint64_t vAddr) {
    if (vAddr > VStackEnd || vAddr < VStackStart) {
        return E_INVALID_BASE_PTR;
    }

    BP = vAddr;
    return UVM_SUCCESS;
}

/**
 * Pushes a value on top of the stack and validates stack pointer
 * @param val Pointer to source data
 * @param size Size of source data
 * @return On success returns UVM_SUCCESS otherwise an error code
 */
uint32_t MemManager::stackPush(void* val, UVMDataSize size) {
    // Check if the stack increase invalidates the stack pointer
    uint64_t oldSP = SP;
    uint64_t newSP = SP + static_cast<uint32_t>(size);
    if (setStackPtr(newSP) != 0) {
        return E_INVALID_STACK_OP;
    }

    uint64_t bufferOffset = oldSP - VStackStart;
    const uint8_t* stackBuffer = Buffers[StackBufferIndex].Buffer;
    memcpy(const_cast<uint8_t*>(&stackBuffer[bufferOffset]), val,
           static_cast<uint32_t>(size));

    return UVM_SUCCESS;
}

/**
 * Pops a value of the stack
 * @param val Pointer to destination where the popped off value will be stored.
 * If nullptr value will be discarded
 * @param size Size of data to pop
 * @return On success returns UVM_SUCCESS otherwise an error code
 */
uint32_t MemManager::stackPop(uint64_t* out, UVMDataSize size) {
    // Check if the stack increase invalidates the stack pointer
    uint64_t newSP = SP - static_cast<uint32_t>(size);
    if (setStackPtr(newSP) != 0) {
        return E_INVALID_STACK_OP;
    }

    if (out != nullptr) {
        uint64_t bufferOffset = newSP - VStackStart;
        const uint8_t* stackBuffer = Buffers[StackBufferIndex].Buffer;
        memcpy(out, &stackBuffer[bufferOffset], static_cast<uint32_t>(size));
    }

    SP = newSP;
    return UVM_SUCCESS;
}

/**
 * Adds new MemBuffer to memory manager
 * @param vAddr Virtual start address of buffer
 * @param size Buffer size
 * @param type Buffer type
 * @param perm Buffer permissions
 * @return Added buffer index (used for reference)
 */
uint32_t MemManager::addBuffer(uint64_t vAddr,
                               uint32_t size,
                               MemType type,
                               uint8_t perm) {
    uint32_t buffIndex = Buffers.size();
    Buffers.emplace_back(vAddr, size, type, perm);
    return buffIndex;
}

/**
 * Allocates stack and sets stack pointer
 * @param vAddr Virtual start address of stack
 */
void MemManager::initStack() {
    StackBufferIndex = addBuffer(VStackStart, UVM_STACK_SIZE, MemType::STACK,
                                 PERM_READ_MASK | PERM_WRITE_MASK);
    SP = VStackStart;
    VStackEnd = VStackStart + UVM_STACK_SIZE;
}

/**
 * Sets an integer register to a value if input is valid
 * @param id Register id
 * @param val Value to be set
 * @param type Size of data to be set
 * @return On success returns UVM_SUCCESS otherwise returns error code
 */
uint32_t MemManager::setIntReg(uint8_t id, IntVal val, IntType type) {
    switch (id) {
    case REG_INSTR_PTR:
        return E_INVALID_DEST_REG;
        break;
    case REG_STACK_PTR: {
        uint32_t status = setStackPtr(val.I64);
        if (status != 0) {
            return status;
        }
    } break;
    case REG_BASE_PTR: {
        uint32_t status = setBasePtr(val.I64);
        if (status != 0) {
            return status;
        }
    } break;
    case REG_FLAGS:
        return E_INVALID_DEST_REG;
        break;
    default: {
        if (id >= REG_GP_START && id <= REG_GP_END) {
            uint32_t regIndex = id - REG_GP_START;
            switch (type) {
            case IntType::I8:
                GP[regIndex].I8 = val.I8;
                break;
            case IntType::I16:
                GP[regIndex].I16 = val.I16;
                break;
            case IntType::I32:
                GP[regIndex].I32 = val.I32;
                break;
            case IntType::I64:
                GP[regIndex].I64 = val.I64;
                break;
            }
        } else if (id >= REG_FP_START && id <= REG_FP_END) {
            return E_INVALID_TYPE;
        } else {
            return E_INVALID_DEST_REG;
        }
        break;
    }
    }

    return UVM_SUCCESS;
}

/**
 * Sets a float register to a value if input is valid
 * @param id Target register id
 * @param val Floating point value
 * @param type Float data size
 * @return On success returns UVM_SUCCESS otherwise error code
 */
uint32_t MemManager::setFloatReg(uint8_t id, FloatVal val, FloatType type) {
    if (id >= REG_FP_START && id <= REG_FP_END) {
        uint32_t regIndex = id - REG_FP_START;
        switch (type) {
        case FloatType::F32:
            FP[regIndex].F32 = val.F32;
            break;
        case FloatType::F64:
            FP[regIndex].F64 = val.F64;
            break;
        }
    } else if (id == REG_INSTR_PTR || id == REG_STACK_PTR ||
               id == REG_BASE_PTR || (id >= REG_GP_START && id <= REG_GP_END)) {
        return E_INVALID_TYPE;
    } else if (id == REG_FLAGS) {
        return E_INVALID_DEST_REG;
    } else {
        return E_INVALID_DEST_REG;
    }

    return UVM_SUCCESS;
}

/**
 * Gets an integer register value if input is valid
 * @param id Target register id
 * @param val [out] Integer value
 * @return On success returns UVM_SUCCESS otherwise error code
 */
uint32_t MemManager::getIntReg(uint8_t id, IntVal& val) {
    switch (id) {
    case REG_INSTR_PTR:
        val.I64 = IP;
        break;
    case REG_STACK_PTR:
        val.I64 = SP;
        break;
    case REG_BASE_PTR:
        val.I64 = BP;
        break;
    case REG_FLAGS:
        return E_INVALID_SRC_REG;
        break;
    default: {
        if (id >= REG_GP_START && id <= REG_GP_END) {
            uint32_t regIndex = id - REG_GP_START;
            val.I64 = GP[regIndex].I64;
        } else if (id >= REG_FP_START && id <= REG_FP_END) {
            return E_INVALID_TYPE;
        } else {
            return E_INVALID_SRC_REG;
        }
    } break;
    }

    return UVM_SUCCESS;
}

/**
 * Gets a float register value if input is valid
 * @param id Target register id
 * @param val [out] Float value
 * @return On success returns UVM_SUCCESS otherwise error code
 */
uint32_t MemManager::getFloatReg(uint8_t id, FloatVal& val) {
    if (id >= REG_FP_START && id <= REG_FP_END) {
        uint32_t regIndex = id - REG_FP_START;
        val.F64 = FP[regIndex].F64;
    } else if (id == REG_INSTR_PTR || id == REG_STACK_PTR ||
               id == REG_BASE_PTR || (id >= REG_GP_START && id <= REG_GP_END)) {
        return E_INVALID_TYPE;
    } else if (id == REG_FLAGS) {
        return E_INVALID_SRC_REG;
    } else {
        return E_INVALID_SRC_REG;
    }

    return UVM_SUCCESS;
}

/**
 * Turns a valid type byte into a IntType
 * @param type Byte containg the unvalidated type
 * @param Pointer to output IntType
 * @return On valid type byte returns true otherwise false
 */
bool parseIntType(uint8_t type, IntType* intType) {
    switch (type) {
    case 0x1:
        *intType = IntType::I8;
        break;
    case 0x2:
        *intType = IntType::I16;
        break;
    case 0x3:
        *intType = IntType::I32;
        break;
    case 0x4:
        *intType = IntType::I64;
        break;
    default:
        return false;
    }

    return true;
}

/**
 * Turns a valid type byte into a FloatType
 * @param type Byte containg the unvalidated type
 * @param Pointer to output FloatType
 * @return On valid type byte returns true otherwise false
 */
bool parseFloatType(uint8_t type, FloatType* floatType) {
    switch (type) {
    case 0xF0:
        *floatType = FloatType::F32;
        break;
    case 0xF1:
        *floatType = FloatType::F64;
        break;
    default:
        return false;
    }

    return true;
}

/**
 * Validates a register offset and returns the computed address
 * @param buff Pointer to buffer containing register offset
 * @param address Pointer to computed address to be filled out
 * @return On valid register offset returns true otherwise false
 */
bool MemManager::evalRegOffset(uint8_t* buff, uint64_t* address) {
    constexpr uint8_t RO_IR = 0x4F;        // <iR>
    constexpr uint8_t RO_IR_I32 = 0x2F;    // <iR> + <i32>
    constexpr uint8_t RO_IR_IR_I16 = 0x1F; // <iR1> + <iR2> * <i16>

    uint8_t layout = buff[0];
    uint8_t iRegA = buff[1];

    // First register in register offset can only be ip, sp, bp or r0-r15
    IntVal iRegAVal;
    if (getIntReg(iRegA, iRegAVal) != 0) {
        std::cout << "[Runtime] Invalid register in register offset "
                  << std::hex << iRegA << " (iR1)\n";
        return false;
    }

    // Extract possible i16 and i32 values
    uint32_t imm32 = 0;
    uint16_t imm16 = 0;
    memcpy(&imm32, &buff[2], 4);
    memcpy(&imm16, &buff[3], 2);

    // Calculate register offset address
    if (layout == RO_IR) {
        *address = iRegAVal.I64;
    } else if ((layout & RO_IR_I32) == RO_IR_I32) {
        if (layout >> 7 == 0) {
            *address = iRegAVal.I64 + imm32;
        } else {
            *address = iRegAVal.I64 - imm32;
        }
    } else if ((layout & RO_IR_IR_I16) == RO_IR_IR_I16) {
        uint8_t iRegB = buff[2];
        IntVal iRegBVal;
        if (getIntReg(iRegB, iRegBVal) != 0) {
            std::cout << "[Runtime] Invalid register in register offset "
                      << std::hex << iRegB << " (iR2)\n";
            return false;
        }

        if (layout >> 7 == 0) {
            *address = iRegAVal.I64 + iRegBVal.I64 * imm16;
        } else {
            *address = iRegAVal.I64 - iRegBVal.I64 * imm16;
        }
    } else {
        return false;
    }

    return true;
}

/**
 * Allocates a new HeapBlock of given size at the current HeapPointer address
 * @param size HeapBlock size
 * @return On success virtual address of allocated heap block otherwise
 * UVM_NULLPTR
 */
uint64_t MemManager::allocHeap(size_t size) {
    // Actually allocated size is <32-bit size> + <requested_size>
    size_t actualSize = size + 4;

    // Only try to allocate in existing buffer if it can fit into a single
    // MemBuffer
    MemBuffer* hb = nullptr;
    if (actualSize <= HEAP_BLOCK_SIZE) {
        for (MemBuffer& buff : Buffers) {
            if (buff.Type == MemType::HEAP && buff.Capacity >= actualSize) {
                hb = &buff;
                break;
            }
        }
    }

    // If no heap block was found allocate new ones until the size requirment is
    // met
    if (hb == nullptr) {
        uint32_t sizeLeft = actualSize;
        uint64_t allocVAddr = VHeapStart + 4;

        size_t i = 0;
        while (sizeLeft > 0) {
            uint32_t hpId =
                addBuffer(VHeapStart, HEAP_BLOCK_SIZE, MemType::HEAP,
                          PERM_READ_MASK | PERM_WRITE_MASK);
            VHeapStart += HEAP_BLOCK_SIZE;

            if (i == 0) {
                uint32_t* hpAllocSizeTarget =
                    reinterpret_cast<uint32_t*>(Buffers[hpId].Buffer);
                *hpAllocSizeTarget = size;
            }

            if (sizeLeft > HEAP_BLOCK_SIZE) {
                Buffers[hpId].Capacity = 0;
                sizeLeft -= HEAP_BLOCK_SIZE;
            } else {
                Buffers[hpId].Capacity = HEAP_BLOCK_SIZE - sizeLeft;
                sizeLeft = 0;
            }

            i++;
        }

        return allocVAddr;
    } else {
        size_t hbOffset = hb->Size - hb->Capacity;

        // write 32 bit size
        uint8_t* hbBuffer = hb->Buffer;
        uint32_t* hpAllocSizeTarget =
            reinterpret_cast<uint32_t*>(&hbBuffer[hbOffset]);
        *hpAllocSizeTarget = size;

        uint64_t allocAddr = hb->VStartAddr + hbOffset + 4;

        hb->Capacity -= actualSize;
        return allocAddr;
    }

    return UVM_NULLPTR;
}

/**
 * Deallocates a previously allocated heap buffer
 * @param vAddr Virtual start address of memory range to be deallocated
 * @return On sucess returns UVM_SUCCESS otherwise return error status
 */
uint32_t MemManager::deallocHeap(uint64_t vAddr) {
    // Find heap block containing address
    MemBuffer* hb = nullptr;
    size_t hbIndex = 0;
    for (MemBuffer& buff : Buffers) {
        // Has to start at offset 4 to be a valid address which was previously
        // allocated
        if (vAddr >= buff.VStartAddr + 4 &&
            vAddr <= buff.VStartAddr + buff.Size) {
            hb = &buff;
            break;
        }
        hbIndex++;
    }

    if (hb == nullptr) {
        return E_DEALLOC_INVALID_ADDR;
    }

    uint32_t blockSize = 0;
    uint32_t readRes = read(vAddr - 4, &blockSize, UVMDataSize::DWORD, 0);

    if (readRes != UVM_SUCCESS || blockSize == 0) {
        return E_DEALLOC_INVALID_ADDR;
    }

    uint32_t actualBlockSize = blockSize + 4;
    uint32_t sizeLeft = actualBlockSize;
    while (sizeLeft > 0) {
        if (sizeLeft > HEAP_BLOCK_SIZE) {
            sizeLeft -= HEAP_BLOCK_SIZE;
            hb->Freed = hb->Size;
        } else {
            hb->Freed += sizeLeft;
            sizeLeft = 0;
        }

        if (hb->Freed >= hb->Size) {
            // Note: we have to delete the array because std::vectors erase
            // function should but for some reason does not call the destructor
            // of the element to be erased.
            delete[] hb->Buffer;
            Buffers.erase(Buffers.begin() + hbIndex);
        }

        // Because the vector shift one to the left the hbIndex now points to
        // the MemBuffer after the deallocated one
        if (hbIndex < Buffers.size()) {
            hb = &Buffers[hbIndex];
        } else {
            return E_DEALLOC_INVALID_ADDR;
        }
    }

    return UVM_SUCCESS;
}

/**
 * Loads sections from source buffer into memory buffers and sets stack start
 * address
 * @param buff Pointer to source buffer
 * @param size Size of source buffer
 */
void MemManager::loadSections(uint8_t* buff, size_t size) {
    uint64_t Cursor = 0;
    for (const auto& sec : Sections) {
        uint32_t buffIndex =
            addBuffer(sec.VStartAddr, sec.Size, sec.Type, sec.Perm);
        MemBuffer* buffer = &Buffers[buffIndex];
        buffer->read(&buff[sec.VStartAddr]);
        Cursor += sec.VStartAddr + sec.Size;
    }
    VStackStart = Cursor + 1;
}

/**
 * Reads from virtual memory at given address with at least read permission into
 * destination buffer
 * @param vAddr Source virtual address
 * @param dest Pointer to destination buffer of at least given size
 * @param size Size of read
 * @param perm Required permissions of memory section
 */
uint32_t
MemManager::read(uint64_t vAddr, void* dest, UVMDataSize size, uint8_t perm) {
    // Add the read permission
    perm |= PERM_READ_MASK;

    uint32_t sizeBytes = static_cast<uint32_t>(size);

    // TODO: Across multiple buffers
    // Find the buffer of vAddr
    MemBuffer* buffer = nullptr;
    for (MemBuffer& buff : Buffers) {
        if (vAddr >= buff.VStartAddr &&
            vAddr + sizeBytes <= buff.VStartAddr + buff.Size) {
            buffer = &buff;
            break;
        }
    }

    if (buffer == nullptr) {
        return E_VADDR_NOT_FOUND;
    }

    if ((buffer->Perm & perm) != perm) {
        return E_MISSING_PERM;
    }

    size_t buffIndex = vAddr - buffer->VStartAddr;
    memcpy(dest, &buffer->Buffer[buffIndex], sizeBytes);

    return UVM_SUCCESS;
}

/**
 * Reads from virtual memory at given address with at least read permission into
 * destination buffer
 * @param vAddr Source virtual address
 * @param dest Pointer to destination buffer of at least given size
 * @param size Size of read
 * @param perm Required permissions of memory section
 */
uint32_t
MemManager::readLarge(uint64_t vAddr, void* dest, uint32_t size, uint8_t perm) {
    // Add the read permission
    perm |= PERM_READ_MASK;

    // If memory range which should be read spans accross multiple memory
    // buffers it has to perform multiple memcpy's. readLeft contains the size
    // of how much memory is left to be copied and readIndex contains the
    // virtual address of the start of the left memory to be copied.
    int64_t readLeft = size;
    uint64_t readIndex = vAddr;
    while (readLeft > 0) {
        // Find the buffer of vAddr
        MemBuffer* buffer = nullptr;
        for (MemBuffer& buff : Buffers) {
            if (readIndex >= buff.VStartAddr &&
                readIndex < buff.VStartAddr + buff.Size) {
                buffer = &buff;
                break;
            }
        }

        if (buffer == nullptr) {
            return E_VADDR_NOT_FOUND;
        }

        if ((buffer->Perm & perm) != perm) {
            return E_MISSING_PERM;
        }

        uint32_t actualReadSize = size;
        uint64_t buffVEnd = buffer->VStartAddr + buffer->Size;
        if (readIndex + readLeft > buffVEnd) {
            actualReadSize = buffVEnd - readIndex;
        }

        size_t buffIndex = vAddr - buffer->VStartAddr;
        memcpy(dest, &buffer->Buffer[buffIndex], actualReadSize);

        readLeft -= actualReadSize;
        readIndex += actualReadSize;
    }

    return UVM_SUCCESS;
}

/**
 * Writes to virtual memory at given address from source buffer with at least
 * write permission
 * @param src Pointer to source buffer of at least given size
 * @param vAddr Source virtual address
 * @param size Size of read
 * @param perm Required permissions of memory section
 */
uint32_t
MemManager::writeLarge(void* src, uint64_t vAddr, uint32_t size, uint8_t perm) {
    // Add the write permission
    perm |= PERM_WRITE_MASK;

    // If memory range which should be read spans accross multiple memory
    // buffers it has to perform multiple memcpy's. writeLeft contains the size
    // of how much memory is left to be copied and writeIndex contains the
    // virtual address of the start of the left memory to be copied.
    int64_t writeLeft = size;
    uint64_t writeIndex = vAddr;
    while (writeLeft > 0) {
        // Find the buffer of vAddr
        MemBuffer* buffer = nullptr;
        for (MemBuffer& buff : Buffers) {
            if (writeIndex >= buff.VStartAddr &&
                writeIndex < buff.VStartAddr + buff.Size) {
                buffer = &buff;
                break;
            }
        }

        if (buffer == nullptr) {
            return E_VADDR_NOT_FOUND;
        }

        if ((buffer->Perm & perm) != perm) {
            return E_MISSING_PERM;
        }

        uint32_t actualWriteSize = size;
        uint64_t buffVEnd = buffer->VStartAddr + buffer->Size;
        if (writeIndex + writeLeft > buffVEnd) {
            actualWriteSize = buffVEnd - writeIndex;
        }

        size_t buffIndex = vAddr - buffer->VStartAddr;
        memcpy(&buffer->Buffer[buffIndex], src, actualWriteSize);

        writeLeft -= actualWriteSize;
        writeIndex += actualWriteSize;
    }

    return UVM_SUCCESS;
}

/**
 * Writes from source buffer into virtual memory at given address with at least
 * write permission
 * @param src Pointer to source buffer of at least given size
 * @param vAddr Destination virtual address
 * @param size Size of write
 * @param perm Required permissions of memory section
 */
uint32_t
MemManager::write(void* src, uint64_t vAddr, UVMDataSize size, uint8_t perm) {
    // Add the write permission
    perm |= PERM_WRITE_MASK;

    uint32_t sizeBytes = static_cast<uint32_t>(size);

    // TODO: Across multiple buffers
    // Find the buffer of vAddr
    MemBuffer* buffer = nullptr;
    for (MemBuffer& buff : Buffers) {
        if (vAddr >= buff.VStartAddr &&
            vAddr + sizeBytes <= buff.VStartAddr + buff.Size) {
            buffer = &buff;
            break;
        }
    }

    if (buffer == nullptr) {
        return E_VADDR_NOT_FOUND;
    }

    if ((buffer->Perm & perm) != perm) {
        return E_MISSING_PERM;
    }

    size_t buffIndex = vAddr - buffer->VStartAddr;
    memcpy(&buffer->Buffer[buffIndex], src, sizeBytes);

    return UVM_SUCCESS;
}

/**
 * Fetches an instruction at instruction pointer and writes it into dest buffer
 * of size
 * @param dest Pointer to destination buffer of at least given size
 * @param size Size of read
 */
uint32_t MemManager::fetchInstruction(uint8_t* dest, size_t size) {
    // Add the execute permission
    uint8_t perm = PERM_EXE_MASK;

    // TODO: Across multiple buffers
    // Find the buffer of vAddr
    MemBuffer* buffer = nullptr;
    for (MemBuffer& buff : Buffers) {
        if (IP >= buff.VStartAddr && IP + size <= buff.VStartAddr + buff.Size) {
            buffer = &buff;
            break;
        }
    }

    if (buffer == nullptr) {
        return E_VADDR_NOT_FOUND;
    }

    if ((buffer->Perm & perm) != perm) {
        return E_MISSING_PERM;
    }

    size_t buffIndex = IP - buffer->VStartAddr;
    memcpy(dest, &buffer->Buffer[buffIndex], size);

    return UVM_SUCCESS;
}
