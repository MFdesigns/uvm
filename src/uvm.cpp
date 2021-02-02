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

#include "uvm.hpp"
#include "error.hpp"
#include "instr/instructions.hpp"
#include "memory.hpp"
#include <cstring>
#include <fstream>
#include <iostream>

bool validateHeader(HeaderInfo* info, uint8_t* source, size_t size) {
    // Check if source file has minimal size to contain a valid header
    constexpr uint32_t MIN_HEADER_SIZE = 0x60;
    if (size < MIN_HEADER_SIZE) {
        std::cout << "[Error] Invalid file header: header size smaller than "
                     "required to be a valid header\n";
        return false;
    }

    // Validate magic number
    constexpr uint32_t MAGIC = 0x50504953; // Magic 'SIPP' in big endianness
    uint32_t* sourceMagic = reinterpret_cast<uint32_t*>(source);
    if (*sourceMagic != MAGIC) {
        std::cout << "[Error] Invalid magic number inside header\n";
        return false;
    }

    // Check version
    constexpr uint64_t VERSION_OFFSET = 0x04;
    uint8_t version = source[VERSION_OFFSET];
    if (version != 1) {
        std::cout << "[Error] Unsupported file version '" << (uint16_t)version
                  << "'\n";
        return false;
    }
    info->Version = version;

    // Check mode
    constexpr uint64_t MODE_OFFSET = 0x05;
    uint8_t mode = source[MODE_OFFSET];
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
    uint64_t* startAddress = (uint64_t*)&source[START_ADDR_OFFSET];
    // Check if start address point inside source buffer. More in depth
    // validation will be performed later once the section table has been parsed
    if (*startAddress > (uint64_t)size) {
        std::cout << "[Error] Invalid start address: Address points outside of "
                     "source file "
                  << std::hex << "0x" << *startAddress << " \n";
        return false;
    }
    info->StartAddress = *startAddress;

    return true;
}

// TODO: Update section type range
bool parseSectionType(uint8_t type, MemType& secType) {
    // Outside of valid section type range
    if (type < 0x1 || type > 0x5) {
        return false;
    }

    secType = (MemType)type;
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

/**
 * Validates the section table and adds sections to vector
 * @param sections Output vector of sections
 * @param buff Pointer to source buffer
 * @param size Size of source buffer
 * @return On success returns true otherwise false
 */
bool parseSectionTable(std::vector<MemSection>* sections,
                       uint8_t* buff,
                       size_t size) {
    constexpr uint64_t SEC_TABLE_OFFSET = 0x60;

    // Range check if section table size is given
    if (SEC_TABLE_OFFSET + sizeof(uint32_t) > size) {
        std::cout
            << "[Error] Invalid section table: no section table size found\n";
        return false;
    }

    constexpr uint64_t SEC_TABLE_ENTRY_SIZE = 0x16;
    uint32_t* tableSize = (uint32_t*)&buff[SEC_TABLE_OFFSET];

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
    sections->reserve(sectionCount);

    // Start to parse the section table entries
    uint64_t cursor = SEC_TABLE_OFFSET + sizeof(uint32_t);
    uint64_t tableEnd = SEC_TABLE_OFFSET + sizeof(uint32_t) + *tableSize;
    bool validSectionTable = true;
    while (cursor < tableEnd && validSectionTable) {
        uint8_t type = buff[cursor];
        uint8_t perms = buff[cursor + 1];

        auto memType = MemType::STATIC;

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
        std::memcpy(&startAddress, &buff[cursor + SEC_START_ADDR_OFFSET], 8);
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
        uint32_t secSize = 0;
        std::memcpy(&secSize, &buff[cursor + SEC_SIZE_OFFSET], 4);
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
        constexpr uint64_t SEC_NAME_OFFSET = 0xE;
        uint64_t secNameAddress = 0;
        std::memcpy(&secNameAddress, &buff[cursor + SEC_NAME_OFFSET], 8);
        // Check if section name points into file buffer. More validation will
        // be performed at a later stage
        if (secNameAddress > size) {
            std::cout << "[Error] Invalid section name address 0x" << std::hex
                      << secNameAddress << '\n';
            validSectionTable = false;
            continue;
        }

        sections->emplace_back(memType, perms, startAddress, secSize);
        cursor += SEC_TABLE_ENTRY_SIZE;
    }

    return validSectionTable;
}

void UVM::setFilePath(std::filesystem::path p) {
    SourcePath = std::move(p);
}

bool UVM::init() {
    MMU.initStack();

    // Set the start address of the heap memory range
    MMU.VHeapStart = MMU.VStackEnd + 1;

    // TODO: Validate start address
    MMU.IP = HInfo.StartAddress;

    return true;
}

/**
 * Reads source file into ram
 * @param p Path to source file
 * @param size Output file size
 */
uint8_t* UVM::readSource(std::filesystem::path p, size_t* size) {
    // Read source file into buffer
    std::ifstream stream{p};

    // Get buffer size
    stream.seekg(0, std::ios::end);
    *size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    // Allocate new buffer of file size and read complete file to buffer
    uint8_t* buffer = new uint8_t[*size]();
    stream.read((char*)buffer, *size);

    return buffer;
}

/**
 * Loads an UX source file and initializes it
 * @param buff Pointer to source buffer
 * @param size Size of source buffer
 * @return On success return zero otherwise non-zero value
 */
uint32_t UVM::loadFile(uint8_t* buff, size_t size) {
    bool validHeader = validateHeader(&HInfo, buff, size);
    if (!validHeader) {
        return E_INVALID_HEADER;
    }

    bool validSecTable = parseSectionTable(&MMU.Sections, buff, size);
    if (!validSecTable) {
        return E_INVALID_SEC_TABLE;
    }

    MMU.loadSections(buff, size);

    return UVM_SUCCESS;
}

bool UVM::run() {
    bool runtimeError = false;

    while (Opcode != OP_EXIT && !runtimeError) {
        runtimeError = !nextInstr();
    }

    return !runtimeError;
}

bool UVM::nextInstr() {
    uint32_t instrWidth = 1;

    // Get opcode byte
    uint32_t readRes =
        MMU.read(MMU.IP, &Opcode, UVMDataSize::BYTE, PERM_EXE_MASK);
    if (readRes != UVM_SUCCESS) {
        return false;
    }

    uint32_t (*instrCall)(UVM * vm, uint32_t instrWidth, uint32_t flag) =
        nullptr;
    uint32_t instrFlag = 0;

    switch (Opcode) {

    case OP_NOP:
        instrWidth = 1;
        break;

    /********************************
        PUSH INSTRUCTIONS
    ********************************/
    case OP_PUSH_I8:
        instrWidth = 2;
        instrFlag = static_cast<uint32_t>(IntType::I8);
        instrCall = instr_push_int;
        break;
    case OP_PUSH_I16:
        instrWidth = 3;
        instrFlag = static_cast<uint32_t>(IntType::I16);
        instrCall = instr_push_int;
        break;
    case OP_PUSH_I32:
        instrWidth = 5;
        instrFlag = static_cast<uint32_t>(IntType::I32);
        instrCall = instr_push_int;
        break;
    case OP_PUSH_I64:
        instrWidth = 9;
        instrFlag = static_cast<uint32_t>(IntType::I64);
        instrCall = instr_push_int;
        break;
    case OP_PUSH_IT_IR:
        instrWidth = 3;
        instrCall = instr_push_ireg;
        break;

    /********************************
        POP INSTRUCTIONS
    ********************************/
    case OP_POP_IT:
        instrWidth = 2;
        instrCall = instr_pop;
        break;
    case OP_POP_IT_IR:
        instrWidth = 3;
        instrCall = instr_pop_ireg;
        break;

    /********************************
        LOAD INSTRUCTIONS
    ********************************/
    case OP_LOAD_I8_IR:
        instrWidth = 3;
        instrFlag = static_cast<uint32_t>(IntType::I8);
        instrCall = instr_load_int_ireg;
        break;
    case OP_LOAD_I16_IR:
        instrWidth = 4;
        instrFlag = static_cast<uint32_t>(IntType::I16);
        instrCall = instr_load_int_ireg;
        break;
    case OP_LOAD_I32_IR:
        instrWidth = 6;
        instrFlag = static_cast<uint32_t>(IntType::I32);
        instrCall = instr_load_int_ireg;
        break;
    case OP_LOAD_I64_IR:
        instrWidth = 10;
        instrFlag = static_cast<uint32_t>(IntType::I64);
        instrCall = instr_load_int_ireg;
        break;
    case OP_LOAD_IT_RO_IR:
        instrWidth = 9;
        instrCall = instr_load_ro_ireg;
        break;
    case OP_LOAD_F32_FR:
        instrWidth = 6;
        instrFlag = static_cast<uint32_t>(FloatType::F32);
        instrCall = instr_loadf_float_freg;
        break;
    case OP_LOAD_F64_FR:
        instrWidth = 10;
        instrFlag = static_cast<uint32_t>(FloatType::F64);
        instrCall = instr_loadf_float_freg;
        break;
    case OP_LOAD_RO_FR:
        instrWidth = 9;
        instrCall = instr_loadf_ro_freg;
        break;

    /********************************
        STORE INSTRUCTION
    ********************************/
    case OP_STORE_IT_IR_RO:
        instrWidth = 9;
        instrCall = instr_store_ireg_ro;
        break;

    /********************************
        COPY INSTRUCTIONS
    ********************************/
    case OP_COPY_I8_RO:
        instrWidth = 8;
        instrFlag = static_cast<uint32_t>(IntType::I8);
        instrCall = instr_copy_int_ro;
        break;
    case OP_COPY_I16_RO:
        instrWidth = 9;
        instrFlag = static_cast<uint32_t>(IntType::I16);
        instrCall = instr_copy_int_ro;
        break;
    case OP_COPY_I32_RO:
        instrWidth = 11;
        instrFlag = static_cast<uint32_t>(IntType::I32);
        instrCall = instr_copy_int_ro;
        break;
    case OP_COPY_I64_RO:
        instrWidth = 15;
        instrFlag = static_cast<uint32_t>(IntType::I64);
        instrCall = instr_copy_int_ro;
        break;
    case OP_COPY_IT_IR_IR:
        instrWidth = 4;
        instrCall = instr_copy_ireg_ireg;
        break;
    case OP_COPY_IT_RO_RO:
        instrWidth = 14;
        instrCall = instr_copy_ro_ro;
        break;

    /********************************
        ARITHMETIC INSTRUCTIONS
    ********************************/
    case OP_ADD_IT_IR_IR:
        instrWidth = 4;
        instrCall = instr_add_ireg_ireg;
        break;
    case OP_SUB_IT_IR_IR:
        instrWidth = 4;
        instrCall = instr_sub_ireg_ireg;
        break;
    case OP_MUL_IT_IR_IR:
        instrWidth = 4;
        instrCall = instr_mul_ireg_ireg;
        break;
    case OP_DIV_IT_IR_IR:
        instrWidth = 4;
        instrCall = instr_div_ireg_ireg;
        break;

    /********************************
        LEA INSTRUCTION
    ********************************/
    case OP_LEA_RO_IR:
        instrWidth = 8;
        instrCall = instr_lea_ro_ireg;
        break;

    /********************************
        SYSCALL
    ********************************/
    case OP_SYS:
        instrWidth = 2;
        instrCall = instr_syscall;
        break;

    /********************************
        CALL and RET
    ********************************/
    case OP_CALL:
        instrWidth = 9;
        instrCall = instr_call;
        break;
    case OP_RET:
        instrWidth = 1;
        instrCall = instr_ret;
        break;

    /********************************
        CONDITIONS
    ********************************/
    case OP_JMP: {
        instrWidth = 9;
        instrFlag = static_cast<uint32_t>(JumpCondition::UNCONDITIONAL);
        instrCall = instr_jmp;
        break;
    }
    case OP_JE: {
        instrWidth = 9;
        instrFlag = static_cast<uint32_t>(JumpCondition::IF_EQUALS);
        instrCall = instr_jmp;
        break;
    }
    case OP_JNE: {
        instrWidth = 9;
        instrFlag = static_cast<uint32_t>(JumpCondition::IF_NOT_EQUALS);
        instrCall = instr_jmp;
        break;
    }
    case OP_JGT: {
        instrWidth = 9;
        instrFlag = static_cast<uint32_t>(JumpCondition::IF_GREATER_THAN);
        instrCall = instr_jmp;
        break;
    }
    case OP_JLT: {
        instrWidth = 9;
        instrFlag = static_cast<uint32_t>(JumpCondition::IF_LESS_THAN);
        instrCall = instr_jmp;
        break;
    }
    case OP_JGE: {
        instrWidth = 9;
        instrFlag = static_cast<uint32_t>(JumpCondition::IF_GREATER_EQUALS);
        instrCall = instr_jmp;
        break;
    }
    case OP_JLE: {
        instrWidth = 9;
        instrFlag = static_cast<uint32_t>(JumpCondition::IF_LESS_EQUALS);
        instrCall = instr_jmp;
        break;
    }
    case OP_CMP_IT_IR_IR:
        instrWidth = 4;
        instrCall = instr_cmp_ireg_ireg;
        break;

    /********************************
        TYPE CASTING
    ********************************/
    case OP_B2L:
        instrWidth = 2;
        instrFlag = static_cast<uint32_t>(IntType::I8);
        instrCall = instr_unsigned_cast_to_long;
        break;
    case OP_S2L:
        instrWidth = 2;
        instrFlag = static_cast<uint32_t>(IntType::I16);
        instrCall = instr_unsigned_cast_to_long;
        break;
    case OP_I2L:
        instrWidth = 2;
        instrFlag = static_cast<uint32_t>(IntType::I32);
        instrCall = instr_unsigned_cast_to_long;
        break;

    /********************************
        EXIT
    ********************************/
    case OP_EXIT:
        return true;
    default:
        std::cout << "[Runtime] Unknow opcode 0x" << std::hex
                  << (uint16_t)Opcode << '\n';
        return false;
    }

    uint32_t fetchRes =
        MMU.fetchInstruction(MMU.InstrBuffer.data(), instrWidth);
    if (fetchRes != UVM_SUCCESS) {
        return false;
    }

    // If Opcode is NOP then instrCall will be nullptr
    if (instrCall != nullptr) {
        uint32_t instrCallRes = instrCall(this, instrWidth, instrFlag);
        if (instrCallRes != UVM_SUCCESS) {
            if (instrCallRes == UVM_SUCCESS_JUMPED) {
                return true;
            } else {
                return false;
            }
        }
    }

    // TODO: Check IP perm
    MMU.IP += instrWidth;
    return true;
}
