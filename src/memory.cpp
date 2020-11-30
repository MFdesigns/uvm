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
#include <cstring>
#include <iostream>

/**
 * Constructs a new MemBuffer
 * @param startAddr Virtual start address of memory buffer
 * @param size Size in bytes of memory section
 */
MemBuffer::MemBuffer(uint64_t startAddr, uint32_t size)
    : VStartAddr(startAddr), Size(size) {
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
MemSection::MemSection(SectionType type,
                       uint8_t perm,
                       uint64_t startAddr,
                       uint32_t size,
                       uint32_t buffIndex)
    : Type(type), Perm(perm), VStartAddr(startAddr), Size(size),
      BufferIndex(buffIndex) {}

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
 * @return On success returns 0 otherwise error code
 */
uint32_t MemManager::setStackPtr(uint64_t vAddr) {
    if (vAddr > VStackEnd) {
        return ERR_STACK_OVERFLOW;
    } else if (vAddr < VStackStart) {
        return ERR_STACK_UNDERFLOW;
    }

    SP = vAddr;
    return 0;
}

/**
 * Sets the base pointer to the new address if it passes the stack range check
 * @param vAddr New base pointer address
 * @return On success returns 0 otherwise error code
 */
uint32_t MemManager::setBasePtr(uint64_t vAddr) {
    if (vAddr > VStackEnd || vAddr < VStackStart) {
        return ERR_BASE_PTR_OUTSIDE_STACK;
    }
    BP = vAddr;
    return 0;
}

/**
 * Pushes a value on top of the stack and validates stack pointer
 * @param val Pointer to source data
 * @param size Size of source data
 * @return On success returns 0 otherwise an error code
 */
uint32_t MemManager::stackPush(void* val, UVMDataSize size) {
    // Check if the stack increase invalidates the stack pointer
    uint64_t oldSP = SP;
    uint64_t newSP = SP + static_cast<uint32_t>(size);
    if (setStackPtr(newSP) != 0) {
        return ERR_STACK_OVERFLOW;
    }

    uint64_t bufferOffset = oldSP - VStackStart;
    const uint8_t* stackBuffer = Buffers[StackBufferIndex].getBuffer();
    memcpy(const_cast<uint8_t*>(&stackBuffer[bufferOffset]), val,
           static_cast<uint32_t>(size));

    return 0;
}

/**
 * Pops a value of the stack
 * @param val Pointer to destination where the popped off value will be stored.
 * If nullptr value will be discarded
 * @param size Size of data to pop
 * @return On success returns 0 otherwise an error code
 */
uint32_t MemManager::stackPop(uint64_t* out, UVMDataSize size) {
    // Check if the stack increase invalidates the stack pointer
    uint64_t newSP = SP - static_cast<uint32_t>(size);
    if (setStackPtr(newSP) != 0) {
        return ERR_STACK_UNDERFLOW;
    }

    if (out != nullptr) {
        uint64_t bufferOffset = newSP - VStackStart;
        const uint8_t* stackBuffer = Buffers[StackBufferIndex].getBuffer();
        memcpy(out, &stackBuffer[bufferOffset], static_cast<uint32_t>(size));
    }

    SP = newSP;
    return 0;
}

bool MemManager::readPhysicalMem(uint64_t vAddr,
                                 uint32_t size,
                                 uint8_t perms,
                                 uint8_t** ptr) const {
    MemSection* section = findSection(vAddr, size);
    if (section == nullptr) {
        std::cout << "[Error] Failed reading memory at address 0x" << std::hex
                  << vAddr << "\n\tmemory not owned by programm\n";
        return false;
    }

    if ((section->Perm & perms) != perms) {
        std::cout << "[Error] Failed reading memory at address 0x" << std::hex
                  << vAddr << "\n\tmissing read permission\n";
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
        std::cout << "[Error] Failed writing to memory at address 0x"
                  << std::hex << vAddr << '\n';
        return false;
    }

    memcpy(dest, source, size);
    return true;
}

/**
 * Adds new MemBuffer to memory manager
 * @param vAddr Virtual start address of buffer
 * @param size Buffer size
 * @return Added buffer index (used for reference)
 */
uint32_t MemManager::addBuffer(uint64_t vAddr, uint32_t size) {
    uint32_t buffIndex = Buffers.size();
    Buffers.emplace_back(vAddr, size);
    return buffIndex;
}

/**
 * Allocates stack, creates corresponding section and sets stack pointer
 * @param vAddr Virtual start address of stack
 */
void MemManager::initStack(uint64_t vAddr) {
    StackBufferIndex = addBuffer(vAddr, UVM_STACK_SIZE);
    Sections.emplace_back(SectionType::STACK, PERM_WRITE_MASK | PERM_READ_MASK,
                          vAddr, UVM_STACK_SIZE, StackBufferIndex);
    SP = vAddr;
    VStackStart = vAddr;
    VStackEnd = vAddr + UVM_STACK_SIZE;
}

/**
 * Sets an integer register to a value if input is valid
 * @param id Register id
 * @param val Value to be set
 * @param type Size of data to be set
 * @return On success returns 0 otherwise returns error code
 */
uint32_t MemManager::setIntReg(uint8_t id, IntVal val, IntType type) {
    switch (id) {
    case REG_INSTR_PTR:
        return ERR_REG_NO_PERM;
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
        return ERR_REG_NO_PERM;
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
            return ERR_REG_TYPE_MISMATCH;
        } else {
        }
        break;
    }

        return ERR_REG_UNKNOWN_ID;
    }

    return 0;
}

/**
 * Sets a float register to a value if input is valid
 * @param id Target register id
 * @param val Floating point value
 * @param type Float data size
 * @return On success returns 0 otherwise error code
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
        return ERR_REG_TYPE_MISMATCH;
    } else if (id == REG_FLAGS) {
        return ERR_REG_NO_PERM;
    } else {
        return ERR_REG_UNKNOWN_ID;
    }

    return 0;
}

/**
 * Gets an integer register value if input is valid
 * @param id Target register id
 * @param val [out] Integer value
 * @return On success returns 0 otherwise error code
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
    // TODO: Should flags be readable ?
    case REG_FLAGS:
        return ERR_REG_NO_PERM;
        break;
    default: {
        if (id >= REG_GP_START && id <= REG_GP_END) {
            uint32_t regIndex = id - REG_GP_START;
            val.I64 = GP[regIndex].I64;
        } else if (id >= REG_FP_START && id <= REG_FP_END) {
            return ERR_REG_TYPE_MISMATCH;
        } else {
            return ERR_REG_UNKNOWN_ID;
        }
    } break;
    }
    return 0;
}

/**
 * Gets a float register value if input is valid
 * @param id Target register id
 * @param val [out] Float value
 * @return On success returns 0 otherwise error code
 */
uint32_t MemManager::getFloatReg(uint8_t id, FloatVal& val) {
    if (id >= REG_FP_START && id <= REG_FP_END) {
        uint32_t regIndex = id - REG_FP_START;
        FP[regIndex].F64 = val.F64;
    } else if (id == REG_INSTR_PTR || id == REG_STACK_PTR ||
               id == REG_BASE_PTR || (id >= REG_GP_START && id <= REG_GP_END)) {
        return ERR_REG_TYPE_MISMATCH;
    } else if (id == REG_FLAGS) {
        return ERR_REG_NO_PERM;
    } else {
        return ERR_REG_UNKNOWN_ID;
    }

    return 0;
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
