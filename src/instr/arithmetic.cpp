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

#include "arithmetic.hpp"

bool Instr::addIRegToIReg(UVM* vm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, 4, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint8_t type = buff[1];
    uint8_t iRegA = buff[2];
    uint8_t iRegB = buff[3];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    IntVal iRegAVal;
    IntVal iRegBVal;

    if (vm->MMU.getIntReg(iRegA, iRegAVal) != 0) {
        return false;
    }

    if (vm->MMU.getIntReg(iRegB, iRegBVal) != 0) {
        return false;
    }

    IntVal result;
    switch (intType) {
    case IntType::I8:
        result.I8 = iRegAVal.I8 + iRegBVal.I8;
        break;
    case IntType::I16:
        result.I16 = iRegAVal.I16 + iRegBVal.I16;
        break;
    case IntType::I32:
        result.I32 = iRegAVal.I32 + iRegBVal.I32;
        break;
    case IntType::I64:
        result.I64 = iRegAVal.I64 + iRegBVal.I64;
        break;
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setIntReg(iRegB, result, intType);

    return true;
}

bool Instr::subIRegFromIReg(UVM* vm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, 4, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint8_t type = buff[1];
    uint8_t iRegA = buff[2];
    uint8_t iRegB = buff[3];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    IntVal iRegAVal;
    IntVal iRegBVal;

    if (vm->MMU.getIntReg(iRegA, iRegAVal) != 0) {
        return false;
    }

    if (vm->MMU.getIntReg(iRegB, iRegBVal) != 0) {
        return false;
    }

    IntVal result;
    switch (intType) {
    case IntType::I8:
        result.I8 = iRegAVal.I8 - iRegBVal.I8;
        break;
    case IntType::I16:
        result.I16 = iRegAVal.I16 - iRegBVal.I16;
        break;
    case IntType::I32:
        result.I32 = iRegAVal.I32 - iRegBVal.I32;
        break;
    case IntType::I64:
        result.I64 = iRegAVal.I64 - iRegBVal.I64;
        break;
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setIntReg(iRegB, result, intType);

    return true;
}

bool Instr::mulIRegWithIReg(UVM* vm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, 4, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint8_t type = buff[1];
    uint8_t iRegA = buff[2];
    uint8_t iRegB = buff[3];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    IntVal iRegAVal;
    IntVal iRegBVal;

    if (vm->MMU.getIntReg(iRegA, iRegAVal) != 0) {
        return false;
    }

    if (vm->MMU.getIntReg(iRegB, iRegBVal) != 0) {
        return false;
    }

    IntVal result;
    switch (intType) {
    case IntType::I8:
        result.I8 = iRegAVal.I8 * iRegBVal.I8;
        break;
    case IntType::I16:
        result.I16 = iRegAVal.I16 * iRegBVal.I16;
        break;
    case IntType::I32:
        result.I32 = iRegAVal.I32 * iRegBVal.I32;
        break;
    case IntType::I64:
        result.I64 = iRegAVal.I64 * iRegBVal.I64;
        break;
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setIntReg(iRegB, result, intType);

    return true;
}

bool Instr::divIRegByIReg(UVM* vm) {
    // Load complete instruction
    uint8_t* buff = nullptr;
    bool memAccess =
        vm->MMU.readPhysicalMem(vm->MMU.IP, 4, PERM_EXE_MASK, &buff);
    if (!memAccess) {
        return false;
    }

    uint8_t type = buff[1];
    uint8_t iRegA = buff[2];
    uint8_t iRegB = buff[3];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return false;
    }

    IntVal iRegAVal;
    IntVal iRegBVal;

    if (vm->MMU.getIntReg(iRegA, iRegAVal) != 0) {
        return false;
    }

    if (vm->MMU.getIntReg(iRegB, iRegBVal) != 0) {
        return false;
    }

    IntVal result;
    switch (intType) {
    case IntType::I8:
        result.I8 = iRegAVal.I8 / iRegBVal.I8;
        break;
    case IntType::I16:
        result.I16 = iRegAVal.I16 / iRegBVal.I16;
        break;
    case IntType::I32:
        result.I32 = iRegAVal.I32 / iRegBVal.I32;
        break;
    case IntType::I64:
        result.I64 = iRegAVal.I64 / iRegBVal.I64;
        break;
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setIntReg(iRegB, result, intType);

    return true;
}
