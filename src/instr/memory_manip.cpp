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
 * Pushes an integer value of given size on top of the stack and increases the
 * stack pointer by the size of the pushed value
 * @param vm Pointer to current UVM instance
 * @param width Instruction width
 * @param type Operation width deducted from opcode
 * @return On success return true otherwise false
 */
bool Instr::pushInt(UVM* vm, uint32_t width, IntType type) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, width, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    IntVal val;
    UVMDataSize dataSize = UVMDataSize::BYTE;
    switch (type) {
    case IntType::I8:
        val.I8 = buff[1];
        break;
    case IntType::I16:
        val.I16 = *reinterpret_cast<uint16_t*>(&buff[1]);
        dataSize = UVMDataSize::WORD;
        break;
    case IntType::I32:
        val.I32 = *reinterpret_cast<uint32_t*>(&buff[1]);
        dataSize = UVMDataSize::DWORD;
        break;
    case IntType::I64:
        val.I64 = *reinterpret_cast<uint64_t*>(&buff[1]);
        dataSize = UVMDataSize::QWORD;
        break;
    }

    uint32_t status = vm->MMU.stackPush(&val, dataSize);
    if (status != 0) {
        std::cout << "[Runtime] Error code " << std::hex << "0x" << status
                  << '\n';
        return false;
    }

    return true;
}

/**
 * Pushes an integer value of given size from a register on top of the stack and
 * increases the stack pointer by the size of the pushed value
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
bool Instr::pushIReg(UVM* vm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, 3, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint8_t type = buff[1];
    uint8_t iReg = buff[2];

    IntVal intReg{};
    if (vm->MMU.getIntReg(iReg, intReg) != 0) {
        return false;
    }

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    IntVal val;
    UVMDataSize dataSize = UVMDataSize::BYTE;
    switch (intType) {
    case IntType::I8:
        dataSize = UVMDataSize::BYTE;
        break;
    case IntType::I16:
        dataSize = UVMDataSize::WORD;
        break;
    case IntType::I32:
        dataSize = UVMDataSize::DWORD;
        break;
    case IntType::I64:
        dataSize = UVMDataSize::QWORD;
        break;
    }

    uint32_t status = vm->MMU.stackPush(&intReg, dataSize);
    if (status != 0) {
        std::cout << "[Runtime] Error code " << std::hex << "0x" << status
                  << '\n';
        return false;
    }

    return true;
}

/**
 * Decreases the stack pointer by given size
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
bool Instr::pop(UVM* vm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, 2, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint8_t type = buff[1];
    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    UVMDataSize dataSize = UVMDataSize::BYTE;
    switch (intType) {
    case IntType::I8:
        dataSize = UVMDataSize::BYTE;
        break;
    case IntType::I16:
        dataSize = UVMDataSize::WORD;
        break;
    case IntType::I32:
        dataSize = UVMDataSize::DWORD;
        break;
    case IntType::I64:
        dataSize = UVMDataSize::QWORD;
        break;
    }

    uint32_t stackStatus = vm->MMU.stackPop(nullptr, dataSize);
    if (stackStatus != 0) {
        std::cout << "Stack error: 0x" << std::hex << stackStatus << '\n';
        return false;
    }

    return true;
}

/**
 * Decreases the stack pointer by given size and pops the value into a register
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
bool Instr::popIReg(UVM* vm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, 3, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint8_t type = buff[1];
    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    uint8_t reg = buff[2];

    UVMDataSize dataSize = UVMDataSize::BYTE;
    switch (intType) {
    case IntType::I8:
        dataSize = UVMDataSize::BYTE;
        break;
    case IntType::I16:
        dataSize = UVMDataSize::WORD;
        break;
    case IntType::I32:
        dataSize = UVMDataSize::DWORD;
        break;
    case IntType::I64:
        dataSize = UVMDataSize::QWORD;
        break;
    }

    IntVal stackVal;
    uint32_t stackStatus = vm->MMU.stackPop(&stackVal.I64, dataSize);
    if (stackStatus != 0) {
        std::cout << "Stack error: 0x" << std::hex << stackStatus << '\n';
        return false;
    }

    // TODO: Handle errors
    if (vm->MMU.setIntReg(reg, stackVal, intType)) {
        return false;
    }

    return true;
}

/**
 * Loads an immedate integer value into an integer register
 * @param vm Pointer to current UVM instance
 * @param width Instruction width
 * @param type Operation width deducted from opcode
 * @return On success return true otherwise false
 */
bool Instr::loadIntToIReg(UVM* vm, uint32_t width, IntType type) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, width, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    IntVal val;
    switch (type) {
    case IntType::I8:
        val.I8 = buff[1];
        break;
    case IntType::I16:
        std::memcpy(&val.I16, &buff[1], 2);
        break;
    case IntType::I32:
        std::memcpy(&val.I32, &buff[1], 4);
        break;
    case IntType::I64:
        std::memcpy(&val.I64, &buff[1], 8);
        break;
    }

    // Target register is at the last byte in instruction
    uint8_t reg = buff[width - 1];
    if (vm->MMU.setIntReg(reg, val, type) != 0) {
        return false;
    }

    return true;
}

/**
 * Loads an integer value from address at register offset into an integer
 * register
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
bool Instr::loadROToIReg(UVM* vm, uint32_t width) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, width, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    constexpr uint32_t RO_OFFSET = 2;
    uint8_t type = buff[1];
    uint8_t iReg = buff[8];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&buff[RO_OFFSET], &roAddress)) {
        return false;
    }

    uint32_t readSize = 0;
    switch (intType) {
    case IntType::I8:
        readSize = 1;
        break;
    case IntType::I16:
        readSize = 2;
        break;
    case IntType::I32:
        readSize = 4;
        break;
    case IntType::I64:
        readSize = 8;
        break;
    }

    uint8_t* readBuff = nullptr;
    bool readSuccess =
        vm->MMU.readPhysicalMem(roAddress, readSize, PERM_READ_MASK, &readBuff);
    if (!readSuccess) {
        return false;
    }

    IntVal intVal;
    switch (intType) {
    case IntType::I8:
        intVal.I8 = readBuff[0];
        break;
    case IntType::I16:
        std::memcpy(&intVal.I16, &readBuff[0], 2);
        break;
    case IntType::I32:
        std::memcpy(&intVal.I32, &readBuff[0], 4);
        break;
    case IntType::I64:
        std::memcpy(&intVal.I64, &readBuff[0], 8);
        break;
    }

    if (vm->MMU.setIntReg(iReg, intVal, intType) != 0) {
        return false;
    }

    return true;
}

/**
 * Stores an integer register value to the address at register offset
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
bool Instr::storeIRegToRO(UVM* vm) {
    // Load complete instruction
    constexpr uint32_t INSTR_WIDTH = 9;
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, INSTR_WIDTH, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint8_t type = buff[1];
    uint8_t iReg = buff[2];
    constexpr uint32_t RO_OFFSET = 3;

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    IntVal intReg{};
    if (vm->MMU.getIntReg(iReg, intReg) != 0) {
        return false;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&buff[RO_OFFSET], &roAddress)) {
        return false;
    }

    uint32_t writeSize = 0;
    switch (intType) {
    case IntType::I8:
        writeSize = 1;
        break;
    case IntType::I16:
        writeSize = 2;
        break;
    case IntType::I32:
        writeSize = 4;
        break;
    case IntType::I64:
        writeSize = 8;
        break;
    }

    bool writeSuccess = vm->MMU.writePhysicalMem(&intReg, roAddress, writeSize);
    if (!writeSuccess) {
        return false;
    }

    return true;
}

/**
 * Copies immediate integer value to address at register offset
 * @param vm Pointer to current UVM instance
 * @param width Instruction width
 * @param type Operation width deducted from opcode
 * @return On success return true otherwise false
 */
bool Instr::copyIntToRO(UVM* vm, uint32_t width, IntType type) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, width, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    IntVal val;
    uint32_t roOffset = 0;
    uint32_t intSize = 0;
    switch (type) {
    case IntType::I8:
        val.I8 = buff[1];
        intSize = 1;
        roOffset = 2;
        break;
    case IntType::I16:
        std::memcpy(&val.I16, &buff[1], 2);
        intSize = 2;
        roOffset = 3;
        break;
    case IntType::I32:
        std::memcpy(&val.I32, &buff[1], 4);
        intSize = 4;
        roOffset = 5;
        break;
    case IntType::I64:
        std::memcpy(&val.I64, &buff[1], 8);
        intSize = 8;
        roOffset = 9;
        break;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&buff[roOffset], &roAddress)) {
        return false;
    }

    bool writeSuccess = vm->MMU.writePhysicalMem(&val.I64, roAddress, intSize);
    if (!writeSuccess) {
        return false;
    }

    return true;
}

/**
 * Copies source integer register value to destination register
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
bool Instr::copyIRegToIReg(UVM* vm) {
    // Load complete instruction
    constexpr uint32_t INSTR_SIZE = 4;
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, INSTR_SIZE, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    // Get all instruction arguments
    uint8_t type = buff[1];
    uint8_t iRegA = buff[2];
    uint8_t iRegB = buff[3];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    IntVal intRegA;
    if (vm->MMU.getIntReg(iRegA, intRegA) != 0) {
        return false;
    }

    if (vm->MMU.setIntReg(iRegB, intRegA, intType) != 0) {
        return false;
    }

    return true;
}

/**
 * Copies integer value at source register offset to address at destination
 * register offset
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
bool Instr::copyROToRO(UVM* vm) {
    // Load complete instruction
    constexpr uint32_t INSTR_WIDTH = 14;
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, INSTR_WIDTH, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint8_t type = buff[1];
    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    uint32_t readSize = 0;
    switch (intType) {
    case IntType::I8:
        readSize = 1;
        break;
    case IntType::I16:
        readSize = 2;
        break;
    case IntType::I32:
        readSize = 4;
        break;
    case IntType::I64:
        readSize = 8;
        break;
    }

    constexpr uint32_t RO_OFFSET_A = 2;
    constexpr uint32_t RO_OFFSET_B = 8;

    uint64_t roAddrA = 0;
    uint64_t roAddrB = 0;
    if (!vm->MMU.evalRegOffset(&buff[RO_OFFSET_A], &roAddrA) ||
        !vm->MMU.evalRegOffset(&buff[RO_OFFSET_B], &roAddrB)) {
        return false;
    }

    uint8_t* readBuff = nullptr;
    bool readSuccess =
        vm->MMU.readPhysicalMem(roAddrA, readSize, PERM_READ_MASK, &readBuff);
    if (!readSuccess) {
        return false;
    }

    bool writeSuccess = vm->MMU.writePhysicalMem(readBuff, roAddrB, readSize);
    if (!writeSuccess) {
        return false;
    }

    return true;
}

/**
 * Loads the computed address of register offset into integer register
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
bool Instr::leaROToIReg(UVM* vm) {
    // Load complete instruction
    constexpr uint32_t INSTR_WIDTH = 8;
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, INSTR_WIDTH, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint8_t iReg = buff[7];

    constexpr uint32_t RO_OFFSET = 1;
    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&buff[RO_OFFSET], &roAddress)) {
        return false;
    }

    IntVal val;
    val.I64 = roAddress;
    if (vm->MMU.setIntReg(iReg, val, IntType::I64)) {
        return false;
    }

    return true;
}
