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

#include "../error.hpp"
#include "instructions.hpp"
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
uint32_t Instr::pushInt(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t INT_OFFSET = 1;

    IntType type = static_cast<IntType>(flag);
    IntVal val;
    UVMDataSize dataSize = UVMDataSize::BYTE;
    switch (type) {
    case IntType::I8:
        val.I8 = vm->MMU.InstrBuffer[INT_OFFSET];
        break;
    case IntType::I16:
        val.I16 =
            *reinterpret_cast<uint16_t*>(&vm->MMU.InstrBuffer[INT_OFFSET]);
        dataSize = UVMDataSize::WORD;
        break;
    case IntType::I32:
        val.I32 =
            *reinterpret_cast<uint32_t*>(&vm->MMU.InstrBuffer[INT_OFFSET]);
        dataSize = UVMDataSize::DWORD;
        break;
    case IntType::I64:
        val.I64 =
            *reinterpret_cast<uint64_t*>(&vm->MMU.InstrBuffer[INT_OFFSET]);
        dataSize = UVMDataSize::QWORD;
        break;
    }

    uint32_t status = vm->MMU.stackPush(&val, dataSize);
    if (status != 0) {
        std::cout << "[Runtime] Error code " << std::hex << "0x" << status
                  << '\n';
        return 0xFF;
    }

    return UVM_SUCCESS;
}

/**
 * Pushes an integer value of given size from a register on top of the stack and
 * increases the stack pointer by the size of the pushed value
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
uint32_t Instr::pushIReg(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_OFFSET = 2;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t iReg = vm->MMU.InstrBuffer[IREG_OFFSET];

    IntVal intReg{};
    if (vm->MMU.getIntReg(iReg, intReg) != 0) {
        return 0xFF;
    }

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
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
        return 0xFF;
    }

    return UVM_SUCCESS;
}

/**
 * Decreases the stack pointer by given size
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
uint32_t Instr::pop(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
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
        return 0xFF;
    }

    return UVM_SUCCESS;
}

/**
 * Decreases the stack pointer by given size and pops the value into a register
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
uint32_t Instr::popIReg(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_OFFSET = 2;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t reg = vm->MMU.InstrBuffer[IREG_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
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

    IntVal stackVal;
    uint32_t stackStatus = vm->MMU.stackPop(&stackVal.I64, dataSize);
    if (stackStatus != 0) {
        std::cout << "Stack error: 0x" << std::hex << stackStatus << '\n';
        return 0xFF;
    }

    // TODO: Handle errors
    if (vm->MMU.setIntReg(reg, stackVal, intType)) {
        return 0xFF;
    }

    return UVM_SUCCESS;
}

/**
 * Loads an immedate integer value into an integer register
 * @param vm Pointer to current UVM instance
 * @param width Instruction width
 * @param type Operation width deducted from opcode
 * @return On success return true otherwise false
 */
uint32_t Instr::loadIntToIReg(UVM* vm, uint32_t width, uint32_t flag) {
    // TODO: Should be int offset
    constexpr uint32_t IREG_OFFSET = 1;

    IntType intType = static_cast<IntType>(flag);
    IntVal val;
    switch (intType) {
    case IntType::I8:
        val.I8 = vm->MMU.InstrBuffer[IREG_OFFSET];
        break;
    case IntType::I16:
        std::memcpy(&val.I16, &vm->MMU.InstrBuffer[IREG_OFFSET], 2);
        break;
    case IntType::I32:
        std::memcpy(&val.I32, &vm->MMU.InstrBuffer[IREG_OFFSET], 4);
        break;
    case IntType::I64:
        std::memcpy(&val.I64, &vm->MMU.InstrBuffer[IREG_OFFSET], 8);
        break;
    }

    // Target register is at the last byte in instruction
    uint8_t reg = vm->MMU.InstrBuffer[width - 1];
    if (vm->MMU.setIntReg(reg, val, intType) != 0) {
        return 0xFF;
    }

    return UVM_SUCCESS;
}

/**
 * Loads an integer value from address at register offset into an integer
 * register
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
uint32_t Instr::loadROToIReg(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t RO_OFFSET = 2;
    constexpr uint32_t IREG_OFFSET = 8;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t iReg = vm->MMU.InstrBuffer[IREG_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[RO_OFFSET], &roAddress)) {
        return 0xFF;
    }

    UVMDataSize readSize = UVMDataSize::BYTE;
    switch (intType) {
    case IntType::I8:
        readSize = UVMDataSize::BYTE;
        break;
    case IntType::I16:
        readSize = UVMDataSize::WORD;
        break;
    case IntType::I32:
        readSize = UVMDataSize::DWORD;
        break;
    case IntType::I64:
        readSize = UVMDataSize::QWORD;
        break;
    }

    uint64_t readBuff = 0;
    uint32_t readRes = vm->MMU.read(
        roAddress, reinterpret_cast<uint8_t*>(&readBuff), readSize, 0);
    if (readRes != UVM_SUCCESS) {
        return 0xFF;
    }

    IntVal intVal;
    switch (intType) {
    case IntType::I8:
        intVal.I8 = *reinterpret_cast<uint8_t*>(&readBuff);
        break;
    case IntType::I16:
        std::memcpy(&intVal.I16, &readBuff, 2);
        break;
    case IntType::I32:
        std::memcpy(&intVal.I32, &readBuff, 4);
        break;
    case IntType::I64:
        std::memcpy(&intVal.I64, &readBuff, 8);
        break;
    }

    if (vm->MMU.setIntReg(iReg, intVal, intType) != 0) {
        return 0xFF;
    }

    return UVM_SUCCESS;
}

/**
 * Loads immediate float into float register
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag FloatType of instruction version
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_TARGET_REG]
 */
uint32_t Instr::loadf_float_reg(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // loadf <f32> <fR>
    // loadf <f64> <fR>

    constexpr uint32_t FLOAT_OFFSET = 1;

    FloatType floatType = static_cast<FloatType>(flag);
    FloatVal val;
    switch (floatType) {
    case FloatType::F32:
        std::memcpy(&val.F32, &vm->MMU.InstrBuffer[FLOAT_OFFSET], 4);
        break;
    case FloatType::F64:
        std::memcpy(&val.F64, &vm->MMU.InstrBuffer[FLOAT_OFFSET], 8);
        break;
    }

    uint8_t reg = vm->MMU.InstrBuffer[width - 1];
    if (vm->MMU.setFloatReg(reg, val, floatType) != 0) {
        return E_INVALID_TARGET_REG;
    }

    return UVM_SUCCESS;
}

/**
 * Loads float at register offset into float register
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_REG_OFFSET, E_INVALID_READ, E_INVALID_TARGET_REG]
 */
uint32_t Instr::loadf_ro_reg(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // loadf <fT> <RO> <fR>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t RO_OFFSET = 2;
    constexpr uint32_t FREG_OFFSET = 8;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t fReg = vm->MMU.InstrBuffer[FREG_OFFSET];

    FloatType floatType = FloatType::F32;
    if (!parseFloatType(type, &floatType)) {
        return E_INVALID_TYPE;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[RO_OFFSET], &roAddress)) {
        return E_INVALID_REG_OFFSET;
    }

    UVMDataSize readSize = UVMDataSize::BYTE;
    switch (floatType) {
    case FloatType::F32:
        readSize = UVMDataSize::DWORD;
        break;
    case FloatType::F64:
        readSize = UVMDataSize::QWORD;
        break;
    }

    uint64_t readBuff = 0;
    uint32_t readRes = vm->MMU.read(
        roAddress, reinterpret_cast<uint8_t*>(&readBuff), readSize, 0);
    if (readRes != UVM_SUCCESS) {
        return E_INVALID_READ;
    }

    FloatVal floatVal;
    switch (floatType) {
    case FloatType::F32:
        std::memcpy(&floatVal.F32, &readBuff, 4);
        break;
    case FloatType::F64:
        std::memcpy(&floatVal.F64, &readBuff, 8);
        break;
    }

    if (vm->MMU.setFloatReg(fReg, floatVal, floatType) != 0) {
        return E_INVALID_TARGET_REG;
    }

    return UVM_SUCCESS;
}

/**
 * Stores an integer register value to the address at register offset
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
uint32_t Instr::storeIRegToRO(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_OFFSET = 2;
    constexpr uint32_t RO_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t iReg = vm->MMU.InstrBuffer[IREG_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
    }

    IntVal intReg{};
    if (vm->MMU.getIntReg(iReg, intReg) != 0) {
        return 0xFF;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[RO_OFFSET], &roAddress)) {
        return 0xFF;
    }

    UVMDataSize writeSize = UVMDataSize::BYTE;
    switch (intType) {
    case IntType::I8:
        writeSize = UVMDataSize::BYTE;
        break;
    case IntType::I16:
        writeSize = UVMDataSize::WORD;
        break;
    case IntType::I32:
        writeSize = UVMDataSize::DWORD;
        break;
    case IntType::I64:
        writeSize = UVMDataSize::QWORD;
        break;
    }

    uint32_t writeRes = vm->MMU.write(&intReg, roAddress, writeSize, 0);
    if (writeRes != UVM_SUCCESS) {
        return 0xFF;
    }

    return UVM_SUCCESS;
}

/**
 * Copies immediate integer value to address at register offset
 * @param vm Pointer to current UVM instance
 * @param width Instruction width
 * @param type Operation width deducted from opcode
 * @return On success return true otherwise false
 */
uint32_t Instr::copyIntToRO(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t INT_OFFSET = 1;

    IntVal val;
    uint32_t roOffset = 0;
    UVMDataSize intSize = UVMDataSize::BYTE;
    IntType type = static_cast<IntType>(flag);
    switch (type) {
    case IntType::I8:
        val.I8 = vm->MMU.InstrBuffer[INT_OFFSET];
        intSize = UVMDataSize::BYTE;
        roOffset = 2;
        break;
    case IntType::I16:
        std::memcpy(&val.I16, &vm->MMU.InstrBuffer[INT_OFFSET], 2);
        intSize = UVMDataSize::WORD;
        roOffset = 3;
        break;
    case IntType::I32:
        std::memcpy(&val.I32, &vm->MMU.InstrBuffer[INT_OFFSET], 4);
        intSize = UVMDataSize::DWORD;
        roOffset = 5;
        break;
    case IntType::I64:
        std::memcpy(&val.I64, &vm->MMU.InstrBuffer[INT_OFFSET], 8);
        intSize = UVMDataSize::QWORD;
        roOffset = 9;
        break;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[roOffset], &roAddress)) {
        return 0xFF;
    }

    uint32_t writeRes = vm->MMU.write(&val.I64, roAddress, intSize, 0);
    if (writeRes != UVM_SUCCESS) {
        return 0xFF;
    }

    return UVM_SUCCESS;
}

/**
 * Copies source integer register value to destination register
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
uint32_t Instr::copyIRegToIReg(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_A_OFFSET = 2;
    constexpr uint32_t IREG_B_OFFSET = 3;

    // Get all instruction arguments
    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t iRegA = vm->MMU.InstrBuffer[IREG_A_OFFSET];
    uint8_t iRegB = vm->MMU.InstrBuffer[IREG_B_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
    }

    IntVal intRegA;
    if (vm->MMU.getIntReg(iRegA, intRegA) != 0) {
        return 0xFF;
    }

    if (vm->MMU.setIntReg(iRegB, intRegA, intType) != 0) {
        return 0xFF;
    }

    return UVM_SUCCESS;
}

/**
 * Copies integer value at source register offset to address at destination
 * register offset
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
uint32_t Instr::copyROToRO(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t RO_OFFSET_A = 2;
    constexpr uint32_t RO_OFFSET_B = 8;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return 0xFF;
    }

    UVMDataSize readSize = UVMDataSize::BYTE;
    switch (intType) {
    case IntType::I8:
        readSize = UVMDataSize::BYTE;
        break;
    case IntType::I16:
        readSize = UVMDataSize::WORD;
        break;
    case IntType::I32:
        readSize = UVMDataSize::DWORD;
        break;
    case IntType::I64:
        readSize = UVMDataSize::QWORD;
        break;
    }

    uint64_t roAddrA = 0;
    uint64_t roAddrB = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[RO_OFFSET_A], &roAddrA) ||
        !vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[RO_OFFSET_B], &roAddrB)) {
        return 0xFF;
    }

    uint64_t readBuff = 0;
    uint32_t readRes = vm->MMU.read(
        roAddrA, reinterpret_cast<uint8_t*>(&readBuff), readSize, 0);
    if (readRes != UVM_SUCCESS) {
        return 0xFF;
    }

    uint32_t writeRes = vm->MMU.write(&readBuff, roAddrB, readSize, 0);
    if (writeRes != UVM_SUCCESS) {
        return 0xFF;
    }

    return UVM_SUCCESS;
}

// TODO: Update docs
/**
 * Loads the computed address of register offset into integer register
 * @param vm Pointer to current UVM instance
 * @return On success return true otherwise false
 */
uint32_t Instr::leaROToIReg(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t RO_OFFSET = 1;
    constexpr uint32_t IREG_OFFSET = 7;

    uint8_t iReg = vm->MMU.InstrBuffer[IREG_OFFSET];

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[RO_OFFSET], &roAddress)) {
        return 0xFF;
    }

    IntVal val;
    val.I64 = roAddress;
    if (vm->MMU.setIntReg(iReg, val, IntType::I64)) {
        return 0xFF;
    }

    return UVM_SUCCESS;
}
