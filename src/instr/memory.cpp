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

#include "memory.hpp"
#include "../register.hpp"
#include <cstring>

bool Instr::copyIntToRO(UVM* vm,
                        RegisterManager* rm,
                        uint32_t width,
                        IntType type) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->getMem(rm->internalGetIP(), width, PERM_EXE_MASK, &buff);
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
    if (!rm->evalRegOffset(&buff[roOffset], &roAddress)) {
        return false;
    }

    bool writeSuccess = vm->memWrite(&val.I64, roAddress, intSize);
    if (!writeSuccess) {
        return false;
    }

    return true;
}

bool Instr::copyIRegToIReg(UVM* vm, RegisterManager* rm) {
    // Load complete instruction
    constexpr uint32_t INSTR_SIZE = 4;
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->getMem(rm->internalGetIP(), INSTR_SIZE, PERM_EXE_MASK, &buff);
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

    IntVal intRegA{};
    if (!rm->getIntReg(iRegA, intType, &intRegA)) {
        return false;
    }

    UVMInt iRegAInt{intType, intRegA};
    if (!rm->setIntReg(iRegB, &iRegAInt)) {
        return false;
    }

    return true;
}

bool Instr::copyROToRO(UVM* vm, RegisterManager* rm) {
    // Load complete instruction
    constexpr uint32_t INSTR_WIDTH = 14;
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->getMem(rm->internalGetIP(), INSTR_WIDTH, PERM_EXE_MASK, &buff);
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
    if (!rm->evalRegOffset(&buff[RO_OFFSET_A], &roAddrA) ||
        !rm->evalRegOffset(&buff[RO_OFFSET_B], &roAddrB)) {
        return false;
    }

    uint8_t* readBuff = nullptr;
    bool readSuccess = vm->getMem(roAddrA, readSize, PERM_READ_MASK, &readBuff);
    if (!readSuccess) {
        return false;
    }

    bool writeSuccess = vm->memWrite(readBuff, roAddrB, readSize);
    if (!writeSuccess) {
        return false;
    }

    return true;
}

bool Instr::loadIntToIReg(UVM* vm,
                          RegisterManager* rm,
                          uint32_t width,
                          IntType type) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->getMem(rm->internalGetIP(), width, PERM_EXE_MASK, &buff);
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
    UVMInt intVal{type, val};
    uint8_t reg = buff[width - 1];
    if (!rm->setIntReg(reg, &intVal)) {
        return false;
    }

    return true;
}

bool Instr::loadROToIReg(UVM* vm, RegisterManager* rm, uint32_t width) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->getMem(rm->internalGetIP(), width, PERM_EXE_MASK, &buff);
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
    if (!rm->evalRegOffset(&buff[RO_OFFSET], &roAddress)) {
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
        vm->getMem(roAddress, readSize, PERM_READ_MASK, &readBuff);
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

    UVMInt val{intType, intVal};
    bool setIntRegSuccess = rm->setIntReg(iReg, &val);
    if (!setIntRegSuccess) {
        return false;
    }

    return true;
}
