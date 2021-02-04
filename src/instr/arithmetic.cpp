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
#include <cmath>

/**
 * Performs operations for instructions add, sub, mul, muls, div and divs with
 * arguments <ireg> <ireg>
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Type of INSTR_FLAG_OP_* flag
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SOURCE_REG, E_INVALID_TARGET_REG, E_DIVISON_ZERO]
 */
uint32_t instr_arithm_common_ireg_ireg(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // add <iT> <iR1> <iR2>
    // sub <iT> <iR1> <iR2>
    // mul <iT> <iR1> <iR2>
    // muls <iT> <iR1> <iR2>
    // div <iT> <iR1> <iR2>
    // divs <iT> <iR1> <iR2>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t SRC_REG_OFFSET = 2;
    constexpr uint32_t DEST_REG_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t srcRegId = vm->MMU.InstrBuffer[SRC_REG_OFFSET];
    uint8_t destRegId = vm->MMU.InstrBuffer[DEST_REG_OFFSET];

    IntType intType = IntType::I32;
    if (!parseIntType(type, &intType)) {
        return E_INVALID_TYPE;
    }

    IntVal srcRegVal;
    IntVal destRegVal;

    if (vm->MMU.getIntReg(srcRegId, srcRegVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }
    if (vm->MMU.getIntReg(destRegId, destRegVal) != 0) {
        return E_INVALID_TARGET_REG;
    }

    IntVal result;
    if ((flag & INSTR_FLAG_OP_ADD) != 0) {
        switch (intType) {
        case IntType::I8:
            result.I8 = srcRegVal.I8 + destRegVal.I8;
            break;
        case IntType::I16:
            result.I16 = srcRegVal.I16 + destRegVal.I16;
            break;
        case IntType::I32:
            result.I32 = srcRegVal.I32 + destRegVal.I32;
            break;
        case IntType::I64:
            result.I64 = srcRegVal.I64 + destRegVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_SUB) != 0) {
        switch (intType) {
        case IntType::I8:
            result.I8 = srcRegVal.I8 - destRegVal.I8;
            break;
        case IntType::I16:
            result.I16 = srcRegVal.I16 - destRegVal.I16;
            break;
        case IntType::I32:
            result.I32 = srcRegVal.I32 - destRegVal.I32;
            break;
        case IntType::I64:
            result.I64 = srcRegVal.I64 - destRegVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_MUL) != 0) {
        switch (intType) {
        case IntType::I8:
            result.I8 = srcRegVal.I8 * destRegVal.I8;
            break;
        case IntType::I16:
            result.I16 = srcRegVal.I16 * destRegVal.I16;
            break;
        case IntType::I32:
            result.I32 = srcRegVal.I32 * destRegVal.I32;
            break;
        case IntType::I64:
            result.I64 = srcRegVal.I64 * destRegVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_MULS) != 0) {
        switch (intType) {
        case IntType::I8:
            result.S8 = srcRegVal.S8 * destRegVal.S8;
            break;
        case IntType::I16:
            result.S16 = srcRegVal.S16 * destRegVal.S16;
            break;
        case IntType::I32:
            result.S32 = srcRegVal.S32 * destRegVal.S32;
            break;
        case IntType::I64:
            result.S64 = srcRegVal.S64 * destRegVal.S64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_DIV) != 0) {
        switch (intType) {
        case IntType::I8:
            if (destRegVal.I8 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I8 = srcRegVal.I8 / destRegVal.I8;
            break;
        case IntType::I16:
            if (destRegVal.I16 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I16 = srcRegVal.I16 / destRegVal.I16;
            break;
        case IntType::I32:
            if (destRegVal.I32 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I32 = srcRegVal.I32 / destRegVal.I32;
            break;
        case IntType::I64:
            if (destRegVal.I64 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I64 = srcRegVal.I64 / destRegVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_DIVS) != 0) {
        switch (intType) {
        case IntType::I8:
            if (destRegVal.S8 == 0) {
                return E_DIVISON_ZERO;
            }
            result.S8 = srcRegVal.S8 / destRegVal.S8;
            break;
        case IntType::I16:
            if (destRegVal.S16 == 0) {
                return E_DIVISON_ZERO;
            }
            result.S16 = srcRegVal.S16 / destRegVal.S16;
            break;
        case IntType::I32:
            if (destRegVal.S32 == 0) {
                return E_DIVISON_ZERO;
            }
            result.S32 = srcRegVal.S32 / destRegVal.S32;
            break;
        case IntType::I64:
            if (destRegVal.S64 == 0) {
                return E_DIVISON_ZERO;
            }
            result.S64 = srcRegVal.S64 / destRegVal.S64;
            break;
        }
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setIntReg(destRegId, result, intType);

    return UVM_SUCCESS;
}

/**
 * Performs operations for instructions addf, subf, mulf and divf with arguments
 * <freg> <freg>
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Type of INSTR_FLAG_OP_* flag
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SOURCE_REG, E_INVALID_TARGET_REG, E_DIVISON_ZERO]
 */
uint32_t instr_arithm_common_freg_freg(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // addf <iT> <iR1> <iR2>
    // subf <iT> <iR1> <iR2>
    // mulf <iT> <iR1> <iR2>
    // divf <iT> <iR1> <iR2>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t SRC_REG_OFFSET = 2;
    constexpr uint32_t DEST_REG_OFFSET = 3;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t srcRegId = vm->MMU.InstrBuffer[SRC_REG_OFFSET];
    uint8_t destRegId = vm->MMU.InstrBuffer[DEST_REG_OFFSET];

    FloatType floatType = FloatType::F32;
    if (!parseFloatType(type, &floatType)) {
        return E_INVALID_TYPE;
    }

    FloatVal srcRegVal;
    FloatVal destRegVal;

    if (vm->MMU.getFloatReg(srcRegId, srcRegVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }
    if (vm->MMU.getFloatReg(destRegId, destRegVal) != 0) {
        return E_INVALID_TARGET_REG;
    }

    FloatVal result;
    if ((flag & INSTR_FLAG_OP_ADD) != 0) {
        switch (floatType) {
        case FloatType::F32:
            result.F32 = srcRegVal.F32 + destRegVal.F32;
            break;
        case FloatType::F64:
            result.F64 = srcRegVal.F64 + destRegVal.F64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_SUB) != 0) {
        switch (floatType) {
        case FloatType::F32:
            result.F32 = srcRegVal.F32 - destRegVal.F32;
            break;
        case FloatType::F64:
            result.F64 = srcRegVal.F64 - destRegVal.F64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_MUL) != 0) {
        switch (floatType) {
        case FloatType::F32:
            result.F32 = srcRegVal.F32 * destRegVal.F32;
            break;
        case FloatType::F64:
            result.F64 = srcRegVal.F64 * destRegVal.F64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_DIV) != 0) {
        switch (floatType) {
        case FloatType::F32:
            if (destRegVal.F32 == 0) {
                return E_DIVISON_ZERO;
            }
            result.F32 = srcRegVal.F32 / destRegVal.F32;
            break;
        case FloatType::F64:
            if (destRegVal.F64 == 0) {
                return E_DIVISON_ZERO;
            }
            result.F64 = srcRegVal.F64 / destRegVal.F64;
            break;
        }
    }

    // No need to evaluate reg id because this was already done by the earlier
    // get
    vm->MMU.setFloatReg(destRegId, result, floatType);

    return UVM_SUCCESS;
}

/**
 * Performs operations for instructions add, sub, mul, muls, div and divs with
 * arguments <ireg> <int>
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Bitmask of INSTR_FLAG_TYPE_* and INSTR_FLAG_OP_*
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_SOURCE_REG, E_DIVISON_ZERO]
 */
uint32_t instr_arithm_common_ireg_int(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // add <iR> <i8>
    // add <iR> <i16>
    // add <iR> <i32>
    // add <iR> <i64>
    // sub <iR> <i8>
    // sub <iR> <i16>
    // sub <iR> <i32>
    // sub <iR> <i64>
    // mul <iR> <i8>
    // mul <iR> <i16>
    // mul <iR> <i32>
    // mul <iR> <i64>
    // muls <iR> <i8>
    // muls <iR> <i16>
    // muls <iR> <i32>
    // muls <iR> <i64>
    // div <iR> <i8>
    // div <iR> <i16>
    // div <iR> <i32>
    // div <iR> <i64>

    constexpr uint32_t REG_OFFSET = 1;
    constexpr uint32_t INT_OFFSET = 2;

    uint8_t regId = vm->MMU.InstrBuffer[REG_OFFSET];

    IntVal regVal;
    if (vm->MMU.getIntReg(regId, regVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }

    uint32_t type = flag & INSTR_FLAG_TYPE_MASK;

    IntType intType = IntType::I8;
    IntVal operandVal;
    switch (type) {
    case INSTR_FLAG_TYPE_I8:
        intType = IntType::I8;
        operandVal.I8 = vm->MMU.InstrBuffer[INT_OFFSET];
        break;
    case INSTR_FLAG_TYPE_I16:
        intType = IntType::I16;
        operandVal.I16 =
            *reinterpret_cast<uint16_t*>(&vm->MMU.InstrBuffer[INT_OFFSET]);
        break;
    case INSTR_FLAG_TYPE_I32:
        intType = IntType::I32;
        operandVal.I32 =
            *reinterpret_cast<uint32_t*>(&vm->MMU.InstrBuffer[INT_OFFSET]);
        break;
    case INSTR_FLAG_TYPE_I64:
        intType = IntType::I64;
        operandVal.I64 =
            *reinterpret_cast<uint64_t*>(&vm->MMU.InstrBuffer[INT_OFFSET]);
        break;
    }

    IntVal result;
    if ((flag & INSTR_FLAG_OP_ADD) != 0) {
        switch (type) {
        case INSTR_FLAG_TYPE_I8:
            result.I8 = regVal.I8 + operandVal.I8;
            break;
        case INSTR_FLAG_TYPE_I16:
            result.I16 = regVal.I16 + operandVal.I16;
            break;
        case INSTR_FLAG_TYPE_I32:
            result.I32 = regVal.I32 + operandVal.I32;
            break;
        case INSTR_FLAG_TYPE_I64:
            result.I64 = regVal.I64 + operandVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_SUB) != 0) {
        switch (type) {
        case INSTR_FLAG_TYPE_I8:
            result.I8 = regVal.I8 - operandVal.I8;
            break;
        case INSTR_FLAG_TYPE_I16:
            result.I16 = regVal.I16 - operandVal.I16;
            break;
        case INSTR_FLAG_TYPE_I32:
            result.I32 = regVal.I32 - operandVal.I32;
            break;
        case INSTR_FLAG_TYPE_I64:
            result.I64 = regVal.I64 - operandVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_MUL) != 0) {
        switch (type) {
        case INSTR_FLAG_TYPE_I8:
            result.I8 = regVal.I8 * operandVal.I8;
            break;
        case INSTR_FLAG_TYPE_I16:
            result.I16 = regVal.I16 * operandVal.I16;
            break;
        case INSTR_FLAG_TYPE_I32:
            result.I32 = regVal.I32 * operandVal.I32;
            break;
        case INSTR_FLAG_TYPE_I64:
            result.I64 = regVal.I64 * operandVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_MULS) != 0) {
        switch (type) {
        case INSTR_FLAG_TYPE_I8:
            result.S8 = regVal.S8 * operandVal.S8;
            break;
        case INSTR_FLAG_TYPE_I16:
            result.S16 = regVal.S16 * operandVal.S16;
            break;
        case INSTR_FLAG_TYPE_I32:
            result.S32 = regVal.S32 * operandVal.S32;
            break;
        case INSTR_FLAG_TYPE_I64:
            result.S64 = regVal.S64 * operandVal.S64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_DIV) != 0) {
        switch (type) {
        case INSTR_FLAG_TYPE_I8:
            if (operandVal.I8 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I8 = regVal.I8 / operandVal.I8;
            break;
        case INSTR_FLAG_TYPE_I16:
            if (operandVal.I16 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I16 = regVal.I16 / operandVal.I16;
            break;
        case INSTR_FLAG_TYPE_I32:
            if (operandVal.I32 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I32 = regVal.I32 / operandVal.I32;
            break;
        case INSTR_FLAG_TYPE_I64:
            if (operandVal.I64 == 0) {
                return E_DIVISON_ZERO;
            }
            result.I64 = regVal.I64 / operandVal.I64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_DIVS) != 0) {
        switch (type) {
        case INSTR_FLAG_TYPE_I8:
            if (operandVal.S8 == 0) {
                return E_DIVISON_ZERO;
            }
            result.S8 = regVal.S8 / operandVal.S8;
            break;
        case INSTR_FLAG_TYPE_I16:
            if (operandVal.S16 == 0) {
                return E_DIVISON_ZERO;
            }
            result.S16 = regVal.S16 / operandVal.S16;
            break;
        case INSTR_FLAG_TYPE_I32:
            if (operandVal.S32 == 0) {
                return E_DIVISON_ZERO;
            }
            result.S32 = regVal.S32 / operandVal.S32;
            break;
        case INSTR_FLAG_TYPE_I64:
            if (operandVal.S64 == 0) {
                return E_DIVISON_ZERO;
            }
            result.S64 = regVal.S64 / operandVal.S64;
            break;
        }
    }

    vm->MMU.setIntReg(regId, result, intType);

    return UVM_SUCCESS;
}

/**
 * Performs operations for instructions addf, subf, mulf and divf with arguments
 * <freg> <float>
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Bitmask of INSTR_FLAG_TYPE_* and INSTR_FLAG_OP_*
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_SOURCE_REG]
 */
uint32_t
instr_arithm_common_freg_float(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // addf <fR> <f32>
    // addf <fR> <f64>
    // subf <fR> <f32>
    // subf <fR> <f64>
    // mulf <fR> <f32>
    // mulf <fR> <f64>
    // divf <fR> <f32>
    // divf <fR> <f64>

    constexpr uint32_t REG_OFFSET = 1;
    constexpr uint32_t FLOAT_OFFSET = 2;

    uint8_t regId = vm->MMU.InstrBuffer[REG_OFFSET];

    FloatVal regVal;
    if (vm->MMU.getFloatReg(regId, regVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }

    uint32_t type = flag & INSTR_FLAG_TYPE_MASK;

    FloatType floatType = FloatType::F32;
    FloatVal operandVal;
    switch (type) {
    case INSTR_FLAG_TYPE_F32:
        floatType = FloatType::F32;
        operandVal.F32 =
            *reinterpret_cast<uint32_t*>(&vm->MMU.InstrBuffer[FLOAT_OFFSET]);
        break;
    case INSTR_FLAG_TYPE_F64:
        floatType = FloatType::F64;
        operandVal.F64 =
            *reinterpret_cast<uint64_t*>(&vm->MMU.InstrBuffer[FLOAT_OFFSET]);
        break;
    }

    FloatVal result;
    if ((flag & INSTR_FLAG_OP_ADD) != 0) {
        switch (type) {
        case INSTR_FLAG_TYPE_F32:
            result.F32 = regVal.F32 + operandVal.F32;
            break;
        case INSTR_FLAG_TYPE_F64:
            result.F64 = regVal.F64 + operandVal.F64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_SUB) != 0) {
        switch (type) {
        case INSTR_FLAG_TYPE_F32:
            result.F32 = regVal.F32 - operandVal.F32;
            break;
        case INSTR_FLAG_TYPE_F64:
            result.F64 = regVal.F64 - operandVal.F64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_MUL) != 0) {
        switch (type) {
        case INSTR_FLAG_TYPE_F32:
            result.F32 = regVal.F32 * operandVal.F32;
            break;
        case INSTR_FLAG_TYPE_F64:
            result.F64 = regVal.F64 * operandVal.F64;
            break;
        }
    } else if ((flag & INSTR_FLAG_OP_DIV) != 0) {
        switch (type) {
        case INSTR_FLAG_TYPE_F32:
            result.F32 = regVal.F32 / operandVal.F32;
            break;
        case INSTR_FLAG_TYPE_F64:
            result.F64 = regVal.F64 / operandVal.F64;
            break;
        }
    }

    vm->MMU.setFloatReg(regId, result, floatType);

    return UVM_SUCCESS;
}

/**
 * Computes square root of source register
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag Unused (pass 0)
 * @return On success returns UVM_SUCCESS otherwise error state [E_INVALID_TYPE,
 * E_INVALID_SOURCE_REG]
 */
uint32_t instr_sqrt(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // sqrt <fT> <fR>

    constexpr uint32_t TYPE_OFFSET = 1;
    constexpr uint32_t REG_OFFSET = 2;

    uint8_t type = vm->MMU.InstrBuffer[TYPE_OFFSET];
    uint8_t regId = vm->MMU.InstrBuffer[REG_OFFSET];

    FloatType fType = FloatType::F32;
    if (!parseFloatType(type, &fType)) {
        return E_INVALID_TYPE;
    }

    FloatVal regVal;
    if (vm->MMU.getFloatReg(regId, regVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }

    FloatVal result;
    switch (fType) {
    case FloatType::F32:
        result.F32 = std::sqrt(regVal.F32);
        break;
    case FloatType::F64:
        result.F64 = std::sqrt(regVal.F64);
        break;
    }

    vm->MMU.setFloatReg(regId, result, fType);

    return UVM_SUCCESS;
}

/**
 * Typecasts unsigned i8, i16 or i32 to i64
 * @param vm UVM instance
 * @param width Instruction width
 * @param flag IntType determining instruction version
 * @return On success returns UVM_SUCCESS otherwise error state
 * [E_INVALID_SOURCE_REG]
 */
uint32_t instr_unsigned_cast_to_long(UVM* vm, uint32_t width, uint32_t flag) {
    // Versions:
    // b2l <iR>
    // s2l <iR>
    // i2l <iR>

    constexpr uint32_t IREG_OFFSET = 1;

    constexpr uint64_t I8_MASK = 0xF;
    constexpr uint64_t I16_MASK = 0xFF;
    constexpr uint64_t I32_MASK = 0xFFFF;

    uint8_t srcRegId = vm->MMU.InstrBuffer[IREG_OFFSET];

    IntVal srcRegVal;
    if (vm->MMU.getIntReg(srcRegId, srcRegVal) != 0) {
        return E_INVALID_SOURCE_REG;
    }

    IntType type = static_cast<IntType>(flag);
    switch (type) {
    case IntType::I8:
        srcRegVal.I64 &= I8_MASK;
        break;
    case IntType::I16:
        srcRegVal.I64 &= I16_MASK;
        break;
    case IntType::I32:
        srcRegVal.I64 &= I32_MASK;
        break;
    }

    vm->MMU.setIntReg(srcRegId, srcRegVal, IntType::I64);

    return UVM_SUCCESS;
}
