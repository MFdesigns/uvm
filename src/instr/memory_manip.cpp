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
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag IntType determining instruction version
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_STACK_OPERATION]
 */
uint32_t instr_push_int(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // push <i8>
    // push <i16>
    // push <i32>
    // push <i64>

    constexpr uint32_t INT_OFFSET = 1;

    IntType type = static_cast<IntType>(flag);

    IntVal srcIntVal;
    UVMDataSize dataSize = UVMDataSize::BYTE;
    switch (type) {
    case IntType::I8:
        srcIntVal.I8 = vm->MMU.InstrBuffer[INT_OFFSET];
        break;
    case IntType::I16:
        srcIntVal.I16 =
            *reinterpret_cast<uint16_t*>(&vm->MMU.InstrBuffer[INT_OFFSET]);
        dataSize = UVMDataSize::WORD;
        break;
    case IntType::I32:
        srcIntVal.I32 =
            *reinterpret_cast<uint32_t*>(&vm->MMU.InstrBuffer[INT_OFFSET]);
        dataSize = UVMDataSize::DWORD;
        break;
    case IntType::I64:
        srcIntVal.I64 =
            *reinterpret_cast<uint64_t*>(&vm->MMU.InstrBuffer[INT_OFFSET]);
        dataSize = UVMDataSize::QWORD;
        break;
    }

    uint32_t status = vm->MMU.stackPush(&srcIntVal, dataSize);
    if (status != 0) {
        return E_INVALID_STACK_OPERATION;
    }

    return UVM_SUCCESS;
}

/**
 * Pushes an integer value of given size from a register on top of the stack and
 * increases the stack pointer by the size of the pushed value
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_SOURCE_REG, E_INVALID_TYPE, E_INVALID_STACK_OPERATION]
 */
uint32_t instr_push_ireg(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // push <iT> <iR>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_OFFSET = 2;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t srcRegId = vm->MMU.InstrBuffer[IREG_OFFSET];

    IntVal srcRegVal{};
    if (vm->MMU.getIntReg(srcRegId, srcRegVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return E_INVALID_TYPE;
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

    uint32_t status = vm->MMU.stackPush(&srcRegVal, dataSize);
    if (status != 0) {
        return E_INVALID_STACK_OPERATION;
    }

    return UVM_SUCCESS;
}

/**
 * Decreases the stack pointer by given size
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_STACK_OPERATION]
 */
uint32_t instr_pop(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // pop <iT>

    constexpr uint32_t TYPE_OFFSET = 1;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return E_INVALID_TYPE;
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
        return E_INVALID_STACK_OPERATION;
    }

    return UVM_SUCCESS;
}

/**
 * Decreases the stack pointer by given size and pops the value into a register
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_STACK_OPERATION, E_INVALID_TARGET_REG]
 */
uint32_t instr_pop_ireg(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // pop <iT> <iR>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_OFFSET = 2;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t destRegId = vm->MMU.InstrBuffer[IREG_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return E_INVALID_TYPE;
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
        return E_INVALID_STACK_OPERATION;
    }

    if (vm->MMU.setIntReg(destRegId, stackVal, intType)) {
        return E_INVALID_TARGET_REG;
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
uint32_t instr_load_int_ireg(UVM* vm, uint32_t width, uint32_t flag) {
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
uint32_t instr_load_ro_ireg(UVM* vm, uint32_t width, uint32_t flag) {
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
uint32_t instr_loadf_float_freg(UVM* vm, uint32_t width, uint32_t flag) {
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
uint32_t instr_loadf_ro_freg(UVM* vm, uint32_t width, uint32_t flag) {
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
 * Stores integer from register to address at register offset
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SOURCE_REG, E_INVALID_REG_OFFSET, E_INVALID_WRITE]
 */
uint32_t instr_store_ireg_ro(UVM* vm, uint32_t width, uint32_t flag) {
    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t IREG_OFFSET = 2;
    constexpr uint32_t RO_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t iReg = vm->MMU.InstrBuffer[IREG_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return E_INVALID_TYPE;
    }

    IntVal intReg{};
    if (vm->MMU.getIntReg(iReg, intReg) != 0) {
        return E_INVALID_SOURCE_REG;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[RO_OFFSET], &roAddress)) {
        return E_INVALID_REG_OFFSET;
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
        return E_INVALID_WRITE;
    }

    return UVM_SUCCESS;
}

/**
 * Stores float from register to address at register offset
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SOURCE_REG, E_INVALID_REG_OFFSET, E_INVALID_WRITE]
 */
uint32_t instr_storef_freg_ro(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // storef <fT> <fR> <RO>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t FREG_OFFSET = 2;
    constexpr uint32_t RO_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t sourceReg = vm->MMU.InstrBuffer[FREG_OFFSET];

    FloatType floatType = FloatType::F32;
    if (!parseFloatType(type, &floatType)) {
        return E_INVALID_TYPE;
    }

    FloatVal sourceRegVal{};
    if (vm->MMU.getFloatReg(sourceReg, sourceRegVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[RO_OFFSET], &roAddress)) {
        return E_INVALID_REG_OFFSET;
    }

    UVMDataSize writeSize = UVMDataSize::BYTE;
    switch (floatType) {
    case FloatType::F32:
        writeSize = UVMDataSize::DWORD;
        break;
    case FloatType::F64:
        writeSize = UVMDataSize::QWORD;
        break;
    }

    uint32_t writeRes = vm->MMU.write(&sourceRegVal, roAddress, writeSize, 0);
    if (writeRes != UVM_SUCCESS) {
        return E_INVALID_WRITE;
    }

    return UVM_SUCCESS;
}

/**
 * Copies immediate integer value to address at register offset
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag IntType determines what instruction version is selected
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_REG_OFFSET, E_INVALID_WRITE]
 */
uint32_t instr_copy_int_ro(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // copy <i8> <RO>
    // copy <i16> <RO>
    // copy <i32> <RO>
    // copy <i64> <RO>

    constexpr uint32_t INT_OFFSET = 1;
    IntType type = static_cast<IntType>(flag);

    // Offset of the register offset inside the instruction buffer
    uint32_t roOffset = 0;

    IntVal immVal;
    UVMDataSize intSize = UVMDataSize::BYTE;
    switch (type) {
    case IntType::I8:
        immVal.I8 = vm->MMU.InstrBuffer[INT_OFFSET];
        intSize = UVMDataSize::BYTE;
        roOffset = 2;
        break;
    case IntType::I16:
        std::memcpy(&immVal.I16, &vm->MMU.InstrBuffer[INT_OFFSET], 2);
        intSize = UVMDataSize::WORD;
        roOffset = 3;
        break;
    case IntType::I32:
        std::memcpy(&immVal.I32, &vm->MMU.InstrBuffer[INT_OFFSET], 4);
        intSize = UVMDataSize::DWORD;
        roOffset = 5;
        break;
    case IntType::I64:
        std::memcpy(&immVal.I64, &vm->MMU.InstrBuffer[INT_OFFSET], 8);
        intSize = UVMDataSize::QWORD;
        roOffset = 9;
        break;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[roOffset], &roAddress)) {
        return E_INVALID_REG_OFFSET;
    }

    uint32_t writeRes = vm->MMU.write(&immVal.I64, roAddress, intSize, 0);
    if (writeRes != UVM_SUCCESS) {
        return E_INVALID_WRITE;
    }

    return UVM_SUCCESS;
}

/**
 * Copies source integer register value to destination register
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SOURCE_REG, E_INVALID_TARGET_REG]
 */
uint32_t instr_copy_ireg_ireg(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // copy <iT> <iR1> <iR2>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t SRC_IREG_OFFSET = 2;
    constexpr uint32_t DEST_IREG_OFFSET = 3;

    // Get all instruction arguments
    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t srcRegId = vm->MMU.InstrBuffer[SRC_IREG_OFFSET];
    uint8_t destRegId = vm->MMU.InstrBuffer[DEST_IREG_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return E_INVALID_TYPE;
    }

    IntVal srcRegVal;
    if (vm->MMU.getIntReg(srcRegId, srcRegVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }

    if (vm->MMU.setIntReg(destRegId, srcRegVal, intType) != 0) {
        return E_INVALID_TARGET_REG;
    }

    return UVM_SUCCESS;
}

/**
 * Copies integer value at source register offset to address at destination
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SOURCE_REG_OFFSET, E_INVALID_DEST_REG_OFFSET, E_INVALID_READ,
 * E_INVALID_WRITE]
 */
uint32_t instr_copy_ro_ro(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // copy <iT> <RO1> <RO2>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t SRC_RO_OFFSET = 2;
    constexpr uint32_t DEST_RO_OFFSET = 8;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return E_INVALID_TYPE;
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

    uint64_t srcROAddr = 0;
    uint64_t destROAddr = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[SRC_RO_OFFSET],
                               &srcROAddr)) {
        return E_INVALID_SOURCE_REG_OFFSET;
    }

    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[DEST_RO_OFFSET],
                               &destROAddr)) {
        return E_INVALID_DEST_REG_OFFSET;
    }

    uint64_t readBuff = 0;
    uint32_t readRes = vm->MMU.read(
        srcROAddr, reinterpret_cast<uint8_t*>(&readBuff), readSize, 0);
    if (readRes != UVM_SUCCESS) {
        return E_INVALID_READ;
    }

    uint32_t writeRes = vm->MMU.write(&readBuff, destROAddr, readSize, 0);
    if (writeRes != UVM_SUCCESS) {
        return E_INVALID_WRITE;
    }

    return UVM_SUCCESS;
}

/**
 * Copies immediate float value to address at register offset
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag FloatType determines what instruction version is selected
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_REG_OFFSET, E_INVALID_WRITE]
 */
uint32_t instr_copyf_float_ro(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // copy <f32> <RO>
    // copy <f64> <RO>

    constexpr uint32_t FLOAT_OFFSET = 1;
    FloatType type = static_cast<FloatType>(flag);

    // Offset of the register offset inside the instruction buffer
    uint32_t roOffset = 0;

    FloatVal immVal;
    UVMDataSize intSize = UVMDataSize::BYTE;
    switch (type) {
    case FloatType::F32:
        std::memcpy(&immVal.F32, &vm->MMU.InstrBuffer[FLOAT_OFFSET], 4);
        intSize = UVMDataSize::DWORD;
        roOffset = 5;
        break;
    case FloatType::F64:
        std::memcpy(&immVal.F64, &vm->MMU.InstrBuffer[FLOAT_OFFSET], 8);
        intSize = UVMDataSize::QWORD;
        roOffset = 9;
        break;
    }

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[roOffset], &roAddress)) {
        return E_INVALID_REG_OFFSET;
    }

    uint32_t writeRes = vm->MMU.write(&immVal.F64, roAddress, intSize, 0);
    if (writeRes != UVM_SUCCESS) {
        return E_INVALID_WRITE;
    }

    return UVM_SUCCESS;
}

/**
 * Copies source float register value to destination register
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SOURCE_REG, E_INVALID_TARGET_REG]
 */
uint32_t instr_copyf_freg_freg(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // copy <fT> <fR1> <fR2>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t SRC_FREG_OFFSET = 2;
    constexpr uint32_t DEST_FREG_OFFSET = 3;

    // Get all instruction arguments
    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t srcRegId = vm->MMU.InstrBuffer[SRC_FREG_OFFSET];
    uint8_t destRegId = vm->MMU.InstrBuffer[DEST_FREG_OFFSET];

    FloatType floatType = FloatType::F32;
    if (!parseFloatType(type, &floatType)) {
        return E_INVALID_TYPE;
    }

    FloatVal srcRegVal;
    if (vm->MMU.getFloatReg(srcRegId, srcRegVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }

    if (vm->MMU.setFloatReg(destRegId, srcRegVal, floatType) != 0) {
        return E_INVALID_TARGET_REG;
    }

    return UVM_SUCCESS;
}

/**
 * Copies float value at source register offset to address at destination
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SOURCE_REG_OFFSET, E_INVALID_DEST_REG_OFFSET, E_INVALID_READ,
 * E_INVALID_WRITE]
 */
uint32_t instr_copyf_ro_ro(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // copy <fT> <RO1> <RO2>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t SRC_RO_OFFSET = 2;
    constexpr uint32_t DEST_RO_OFFSET = 8;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];

    FloatType floatType = FloatType::F32;
    if (!parseFloatType(type, &floatType)) {
        return E_INVALID_TYPE;
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

    uint64_t srcROAddr = 0;
    uint64_t destROAddr = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[SRC_RO_OFFSET],
                               &srcROAddr)) {
        return E_INVALID_SOURCE_REG_OFFSET;
    }

    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[DEST_RO_OFFSET],
                               &destROAddr)) {
        return E_INVALID_DEST_REG_OFFSET;
    }

    uint64_t readBuff = 0;
    uint32_t readRes = vm->MMU.read(
        srcROAddr, reinterpret_cast<uint8_t*>(&readBuff), readSize, 0);
    if (readRes != UVM_SUCCESS) {
        return E_INVALID_READ;
    }

    uint32_t writeRes = vm->MMU.write(&readBuff, destROAddr, readSize, 0);
    if (writeRes != UVM_SUCCESS) {
        return E_INVALID_WRITE;
    }

    return UVM_SUCCESS;
}

/**
 * Loads the computed address of register offset into destination integer
 * register
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_SOURCE_REG_OFFSET]
 */
uint32_t instr_lea_ro_ireg(UVM* vm, uint32_t width, uint32_t flag) {
    // Version:
    // lea <RO> <iR>

    constexpr uint32_t RO_OFFSET = 1;
    constexpr uint32_t IREG_OFFSET = 7;

    uint8_t destRegId = vm->MMU.InstrBuffer[IREG_OFFSET];

    uint64_t roAddress = 0;
    if (!vm->MMU.evalRegOffset(&vm->MMU.InstrBuffer[RO_OFFSET], &roAddress)) {
        return E_INVALID_SOURCE_REG_OFFSET;
    }

    IntVal destRegVal;
    destRegVal.I64 = roAddress;
    if (vm->MMU.setIntReg(destRegId, destRegVal, IntType::I64)) {
        return E_INVALID_TARGET_REG;
    }

    return UVM_SUCCESS;
}
