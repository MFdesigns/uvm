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

#include "uvm.hpp"
#include "instr/function.hpp"
#include "instr/memory_manip.hpp"
#include "instr/syscall.hpp"
#include "memory.hpp"
#include <cstring>
#include <fstream>
#include <iostream>

bool validateHeader(HeaderInfo* info, MemBuffer* source) {
    uint8_t* sourceBuffer = source->getBuffer();
    // Check if source file has minimal size to contain a valid header
    constexpr uint32_t MIN_HEADER_SIZE = 0x60;
    if (source->Size < MIN_HEADER_SIZE) {
        std::cout << "[Error] Invalid file header: header size smaller than "
                     "required to be a valid header\n";
        return false;
    }

    // Validate magic number
    constexpr uint32_t MAGIC = 0x50504953; // Magic 'SIPP' in big endianness
    uint32_t* sourceMagic = reinterpret_cast<uint32_t*>(sourceBuffer);
    if (*sourceMagic != MAGIC) {
        std::cout << "[Error] Invalid magic number inside header\n";
        return false;
    }

    // Check version
    constexpr uint64_t VERSION_OFFSET = 0x04;
    uint8_t version = sourceBuffer[VERSION_OFFSET];
    if (version != 1) {
        std::cout << "[Error] Unsupported file version '" << (uint16_t)version
                  << "'\n";
        return false;
    }
    info->Version = version;

    // Check mode
    constexpr uint64_t MODE_OFFSET = 0x05;
    uint8_t mode = sourceBuffer[MODE_OFFSET];
    switch (mode) {
    case 0x1: // Release
    case 0x2: // Debug
        info->Mode = mode;
        break;
    default:
        std::cout << "[Error] Unsupported mode '" << (uint16_t)mode << "'\n";
        return false;
    }

    // Validate start address
    constexpr uint64_t START_ADDR_OFFSET = 0x08;
    uint64_t* startAddress = (uint64_t*)&sourceBuffer[START_ADDR_OFFSET];
    // Check if start address point inside source buffer. More in depth
    // validation will be performed later once the section table has been parsed
    if (*startAddress > (uint64_t)source->Size) {
        std::cout << "[Error] Invalid start address: Address points outside of "
                     "source file "
                  << std::hex << "0x" << *startAddress << " \n";
        return false;
    }
    info->StartAddress = *startAddress;

    return true;
}

bool parseSectionType(uint8_t type, SectionType& secType) {
    // Outside of valid section type range
    if (type < 0x1 || type > 0x5) {
        return false;
    }

    secType = (SectionType)type;
    return true;
}

bool validateSectionPermission(uint8_t perms) {
    constexpr uint8_t UNKNOW_MASK =
        (uint8_t) ~(PERM_READ_MASK | PERM_WRITE_MASK | PERM_EXE_MASK);

    if ((perms & UNKNOW_MASK) != 0) {
        return false;
    }

    return true;
}

bool parseSectionTable(std::vector<MemSection>& sections,
                       MemBuffer* buffer,
                       uint32_t buffIndex) {
    uint64_t size = buffer->Size;
    uint8_t* sourceBuffer = buffer->getBuffer();
    constexpr uint64_t SEC_TABLE_OFFSET = 0x60;

    // Range check if section table size is given
    if (SEC_TABLE_OFFSET + sizeof(uint32_t) > size) {
        std::cout
            << "[Error] Invalid section table: no section table size found\n";
        return false;
    }

    constexpr uint64_t SEC_TABLE_ENTRY_SIZE = 0x1A;
    uint32_t* tableSize = (uint32_t*)&sourceBuffer[SEC_TABLE_OFFSET];

    // Range check given section table size
    if (SEC_TABLE_OFFSET + sizeof(uint32_t) + *tableSize > size) {
        std::cout << "[Error] Invalid section table: given section table size "
                     "is out of range\n";
        return false;
    }

    // Check if there is at least one section table entry
    if (SEC_TABLE_OFFSET + sizeof(uint32_t) + SEC_TABLE_ENTRY_SIZE > size) {
        std::cout << "[Error] Invalid section table: no entries found\n";
        return false;
    }

    // Allocate sections in vector
    uint32_t sectionCount = *tableSize / SEC_TABLE_ENTRY_SIZE;
    sections.reserve(sectionCount);

    // Start to parse the section table entries
    uint64_t cursor = SEC_TABLE_OFFSET + sizeof(uint32_t);
    uint64_t tableEnd = SEC_TABLE_OFFSET + sizeof(uint32_t) + *tableSize;
    bool validSectionTable = true;
    while (cursor < tableEnd && validSectionTable) {
        uint8_t type = sourceBuffer[cursor];
        uint8_t perms = sourceBuffer[cursor + 1];

        auto memType = SectionType::STATIC;

        // Validate section type
        bool validType = parseSectionType(type, memType);
        if (!validType) {
            std::cout << "[Error] Invalid section type 0x" << std::hex
                      << (uint16_t)type << '\n';
            validSectionTable = false;
            continue;
        }

        // Validate section permission and create permission struct
        bool validPerms = validateSectionPermission(perms);
        if (!validPerms) {
            std::cout << "[Error] Invalid section permission 0x" << std::hex
                      << (uint16_t)perms << '\n';
            validSectionTable = false;
            continue;
        }

        // Parse start address and perform basic validation
        constexpr uint64_t SEC_START_ADDR_OFFSET = 0x02;
        uint64_t startAddress = 0;
        std::memcpy(&startAddress,
                    &sourceBuffer[cursor + SEC_START_ADDR_OFFSET], 8);
        // Check if start address points into file buffer. More validation will
        // be performed at a later stage
        if (startAddress > size) {
            std::cout << "[Error] Invalid start address in section entry 0x"
                      << std::hex << startAddress << '\n';
            validSectionTable = false;
            continue;
        }

        // Parse section size and perform basic validation
        constexpr uint64_t SEC_SIZE_OFFSET = 0x0A;
        uint64_t secSize = 0;
        std::memcpy(&secSize, &sourceBuffer[cursor + SEC_SIZE_OFFSET], 8);
        // Check if section size points into file buffer. More validation will
        // be performed at a later stage. Check for 64bit integer overflow
        // before validating section size
        if ((INT64_MAX - startAddress < secSize) ||
            (startAddress + secSize > size)) {
            std::cout << "[Error] Invalid section size 0x" << std::hex
                      << secSize << '\n';
            validSectionTable = false;
            continue;
        }

        // Parse section size and perform basic validation
        constexpr uint64_t SEC_NAME_OFFSET = 0x12;
        uint64_t secNameAddress = 0;
        std::memcpy(&secNameAddress, &sourceBuffer[cursor + SEC_NAME_OFFSET],
                    8);
        // Check if section name points into file buffer. More validation will
        // be performed at a later stage
        if (secNameAddress > size) {
            std::cout << "[Error] Invalid section name address 0x" << std::hex
                      << secNameAddress << '\n';
            validSectionTable = false;
            continue;
        }

        sections.emplace_back(memType, perms, startAddress, secSize, buffIndex);
        cursor += SEC_TABLE_ENTRY_SIZE;
    }

    return validSectionTable;
}

UVM::UVM(std::filesystem::path p)
    : SourcePath(std::move(p)), HInfo(std::make_unique<HeaderInfo>()), MMU() {}

bool UVM::init() {
    readSource();
    bool validHeader =
        validateHeader(HInfo.get(), &MMU.Buffers[SourceBuffIndex]);
    if (!validHeader) {
        return false;
    }

    bool validSectionTable = parseSectionTable(
        MMU.Sections, &MMU.Buffers[SourceBuffIndex], SourceBuffIndex);
    if (!validSectionTable) {
        return false;
    }

    uint64_t vStackStart = MMU.Buffers[SourceBuffIndex].Size + 1;
    MMU.initStack(vStackStart);

    // TODO: Validate start address
    MMU.IP = HInfo->StartAddress;

    return true;
}

void UVM::readSource() {
    // Read source file into buffer
    std::ifstream stream{SourcePath};

    // Get buffer size
    stream.seekg(0, std::ios::end);
    uint64_t size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    SourceBuffIndex = MMU.addBuffer(UVM_START_ADDR, size);

    // Allocate new buffer of file size and read complete file to buffer
    uint8_t* buffer = MMU.Buffers[SourceBuffIndex].getBuffer();
    stream.read((char*)buffer, size);
}

bool UVM::run() {
    uint8_t op = 0;
    bool runtimeError = false;

    while (op != OP_EXIT && !runtimeError) {
        uint32_t instrWidth = 1;

        // Get opcode byte
        uint8_t* opRef = nullptr;
        // Handle invalid memory access
        MMU.readPhysicalMem(MMU.IP, 1, PERM_EXE_MASK, &opRef);
        op = *opRef;

        switch (op) {
        /********************************
            PUSH INSTRUCTIONS
        ********************************/
        case OP_PUSH_I8:
            instrWidth = 2;
            runtimeError = !Instr::pushInt(this, instrWidth, IntType::I8);
            break;
        case OP_PUSH_I16:
            instrWidth = 3;
            runtimeError = !Instr::pushInt(this, instrWidth, IntType::I16);
            break;
        case OP_PUSH_I32:
            instrWidth = 5;
            runtimeError = !Instr::pushInt(this, instrWidth, IntType::I32);
            break;
        case OP_PUSH_I64:
            instrWidth = 9;
            runtimeError = !Instr::pushInt(this, instrWidth, IntType::I64);
            break;
        case OP_PUSH_IT_IR:
            instrWidth = 3;
            runtimeError = !Instr::pushIReg(this);
            break;

        /********************************
            POP INSTRUCTIONS
        ********************************/
        case OP_POP_IT:
            instrWidth = 2;
            runtimeError = !Instr::pop(this);
            break;
        case OP_POP_IT_IR:
            instrWidth = 3;
            runtimeError = !Instr::popIReg(this);
            break;

        /********************************
            LOAD INSTRUCTIONS
        ********************************/
        case OP_LOAD_I8_IR:
            instrWidth = 3;
            runtimeError = !Instr::loadIntToIReg(this, instrWidth, IntType::I8);
            break;
        case OP_LOAD_I16_IR:
            instrWidth = 4;
            runtimeError =
                !Instr::loadIntToIReg(this, instrWidth, IntType::I16);
            break;
        case OP_LOAD_I32_IR:
            instrWidth = 6;
            runtimeError =
                !Instr::loadIntToIReg(this, instrWidth, IntType::I32);
            break;
        case OP_LOAD_I64_IR:
            instrWidth = 10;
            runtimeError =
                !Instr::loadIntToIReg(this, instrWidth, IntType::I64);
            break;
        case OP_LOAD_IT_RO_IR:
            instrWidth = 9;
            runtimeError = !Instr::loadROToIReg(this, instrWidth);
            break;

        /********************************
            STORE INSTRUCTION
        ********************************/
        case OP_STORE_IT_IR_RO:
            instrWidth = 9;
            runtimeError = !Instr::storeIRegToRO(this);
            break;

        /********************************
            COPY INSTRUCTIONS
        ********************************/
        case OP_COPY_I8_RO:
            instrWidth = 8;
            runtimeError = !Instr::copyIntToRO(this, instrWidth, IntType::I8);
            break;
        case OP_COPY_I16_RO:
            instrWidth = 9;
            runtimeError = !Instr::copyIntToRO(this, instrWidth, IntType::I16);
            break;
        case OP_COPY_I32_RO:
            instrWidth = 11;
            runtimeError = !Instr::copyIntToRO(this, instrWidth, IntType::I32);
            break;
        case OP_COPY_I64_RO:
            instrWidth = 15;
            runtimeError = !Instr::copyIntToRO(this, instrWidth, IntType::I64);
            break;
        case OP_COPY_IT_IR_IR:
            instrWidth = 4;
            runtimeError = !Instr::copyIRegToIReg(this);
            break;
        case OP_COPY_IT_RO_RO:
            instrWidth = 14;
            runtimeError = !Instr::copyROToRO(this);
            break;

        /********************************
            LEA INSTRUCTION
        ********************************/
        case OP_LEA_RO_IR:
            instrWidth = 8;
            runtimeError = !Instr::leaROToIReg(this);
            break;

        /********************************
            SYSCALL
        ********************************/
        case OP_SYS:
            instrWidth = 2;
            runtimeError = !Instr::syscall(this);
            break;

        /********************************
            CALL and RET
        ********************************/
        case OP_CALL:
            instrWidth = 9;
            runtimeError = !Instr::call(this);
            continue;
            break;
        case OP_RET:
            instrWidth = 1;
            runtimeError = !Instr::ret(this);
            continue;
            break;

        /********************************
            EXIT
        ********************************/
        case OP_EXIT:
            continue;
        default:
            std::cout << "[Runtime] Unknow opcode 0x" << std::hex
                      << (uint16_t)op << '\n';
            runtimeError = true;
            continue;
        }
        // TODO: Check IP perm
        MMU.IP += instrWidth;
    }

    return !runtimeError;
}
