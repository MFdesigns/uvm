// ======================================================================== //
// Copyright 2021 Michel Fäh
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

#pragma once
#include "../uvm.hpp"
#include <cstdint>

// Instruction opcodes
constexpr uint8_t OP_PUSH_I8 = 0x01;
constexpr uint8_t OP_PUSH_I16 = 0x02;
constexpr uint8_t OP_PUSH_I32 = 0x03;
constexpr uint8_t OP_PUSH_I64 = 0x04;
constexpr uint8_t OP_PUSH_IT_IR = 0x05;
constexpr uint8_t OP_POP_IT = 0x06;
constexpr uint8_t OP_POP_IT_IR = 0x07;
constexpr uint8_t OP_STORE_IT_IR_RO = 0x08;
constexpr uint8_t OP_STORE_FT_FR_RO = 0x09;
constexpr uint8_t OP_LEA_RO_IR = 0x10;
constexpr uint8_t OP_LOAD_I8_IR = 0x11;
constexpr uint8_t OP_LOAD_I16_IR = 0x12;
constexpr uint8_t OP_LOAD_I32_IR = 0x13;
constexpr uint8_t OP_LOAD_I64_IR = 0x14;
constexpr uint8_t OP_LOAD_IT_RO_IR = 0x15;
constexpr uint8_t OP_LOAD_F32_FR = 0x16;
constexpr uint8_t OP_LOAD_F64_FR = 0x17;
constexpr uint8_t OP_LOAD_RO_FR = 0x18;
constexpr uint8_t OP_CALL = 0x20;
constexpr uint8_t OP_COPY_I8_RO = 0x21;
constexpr uint8_t OP_COPY_I16_RO = 0x22;
constexpr uint8_t OP_COPY_I32_RO = 0x23;
constexpr uint8_t OP_COPY_I64_RO = 0x24;
constexpr uint8_t OP_COPY_IT_IR_IR = 0x25;
constexpr uint8_t OP_COPY_IT_RO_RO = 0x26;
constexpr uint8_t OP_COPY_F32_RO = 0x27;
constexpr uint8_t OP_COPY_F64_RO = 0x28;
constexpr uint8_t OP_COPY_FT_FR_FR = 0x29;
constexpr uint8_t OP_COPY_FT_RO_RO = 0x2A;
constexpr uint8_t OP_RET = 0x30;
constexpr uint8_t OP_ADD_IR_I8 = 0x31;
constexpr uint8_t OP_ADD_IR_I16 = 0x32;
constexpr uint8_t OP_ADD_IR_I32 = 0x33;
constexpr uint8_t OP_ADD_IR_I64 = 0x34;
constexpr uint8_t OP_ADD_IT_IR_IR = 0x35;
constexpr uint8_t OP_ADDF_FR_F32 = 0x36;
constexpr uint8_t OP_ADDF_FR_F64 = 0x37;
constexpr uint8_t OP_ADDF_FT_FR_FR = 0x38;
constexpr uint8_t OP_SUB_IR_I8 = 0x41;
constexpr uint8_t OP_SUB_IR_I16 = 0x42;
constexpr uint8_t OP_SUB_IR_I32 = 0x43;
constexpr uint8_t OP_SUB_IR_I64 = 0x44;
constexpr uint8_t OP_SUB_IT_IR_IR = 0x45;
constexpr uint8_t OP_SUBF_FR_F32 = 0x46;
constexpr uint8_t OP_SUBF_FR_F64 = 0x47;
constexpr uint8_t OP_SUBF_FT_FR_FR = 0x48;
constexpr uint8_t OP_MUL_IR_I8 = 0x51;
constexpr uint8_t OP_MUL_IR_I16 = 0x52;
constexpr uint8_t OP_MUL_IR_I32 = 0x53;
constexpr uint8_t OP_MUL_IR_I64 = 0x54;
constexpr uint8_t OP_MUL_IT_IR_IR = 0x55;
constexpr uint8_t OP_MULF_FR_F32 = 0x56;
constexpr uint8_t OP_MULF_FR_F64 = 0x57;
constexpr uint8_t OP_MULF_FT_FR_FR = 0x58;
constexpr uint8_t OP_MULS_IR_I8 = 0x59;
constexpr uint8_t OP_MULS_IR_I16 = 0x5A;
constexpr uint8_t OP_MULS_IR_I32 = 0x5B;
constexpr uint8_t OP_MULS_IR_I64 = 0x5C;
constexpr uint8_t OP_MULS_IT_IR_IR = 0x5D;
constexpr uint8_t OP_DIV_IR_I8 = 0x61;
constexpr uint8_t OP_DIV_IR_I16 = 0x62;
constexpr uint8_t OP_DIV_IR_I32 = 0x63;
constexpr uint8_t OP_DIV_IR_I64 = 0x64;
constexpr uint8_t OP_DIV_IT_IR_IR = 0x65;
constexpr uint8_t OP_DIVF_FR_F32 = 0x66;
constexpr uint8_t OP_DIVF_FR_F64 = 0x67;
constexpr uint8_t OP_DIVF_FT_FR_FR = 0x68;
constexpr uint8_t OP_DIVS_IR_I8 = 0x69;
constexpr uint8_t OP_DIVS_IR_I16 = 0x6A;
constexpr uint8_t OP_DIVS_IR_I32 = 0x6B;
constexpr uint8_t OP_DIVS_IR_I64 = 0x6C;
constexpr uint8_t OP_DIVS_IT_IR_IR = 0x6D;
constexpr uint8_t OP_AND_IT_IR_IR = 0x75;
constexpr uint8_t OP_OR_IT_IR_IR = 0x85;
constexpr uint8_t OP_XOR_IT_IR_IR = 0x95;
constexpr uint8_t OP_NOT_IT_IR = 0xA5;
constexpr uint8_t OP_SYS = 0x40;
constexpr uint8_t OP_EXIT = 0x50;
constexpr uint8_t OP_SQRT = 0x86;
constexpr uint8_t OP_MOD = 0x96;
constexpr uint8_t OP_NOP = 0xA0;
constexpr uint8_t OP_LSH = 0x76;
constexpr uint8_t OP_RSH = 0x77;
constexpr uint8_t OP_SRSH = 0x78;
constexpr uint8_t OP_B2L = 0xB1;
constexpr uint8_t OP_S2L = 0xB2;
constexpr uint8_t OP_I2L = 0xB3;
constexpr uint8_t OP_B2SL = 0xC1;
constexpr uint8_t OP_S2SL = 0xC2;
constexpr uint8_t OP_I2SL = 0xC3;
constexpr uint8_t OP_F2D = 0xB4;
constexpr uint8_t OP_D2F = 0xC4;
constexpr uint8_t OP_I2F = 0xB5;
constexpr uint8_t OP_I2D = 0xC5;
constexpr uint8_t OP_F2I = 0xB6;
constexpr uint8_t OP_D2I = 0xC6;
constexpr uint8_t OP_CMP_IT_IR_IR = 0xD1;
constexpr uint8_t OP_JMP = 0xE1;
constexpr uint8_t OP_JE = 0xE2;
constexpr uint8_t OP_JNE = 0xE3;
constexpr uint8_t OP_JGT = 0xE4;
constexpr uint8_t OP_JLT = 0xE5;
constexpr uint8_t OP_JGE = 0xE6;
constexpr uint8_t OP_JLE = 0xE7;

// Syscalls
constexpr uint8_t SYSCALL_PRINT = 0x1;
constexpr uint8_t SYSCALL_ALLOC = 0x41;
constexpr uint8_t SYSCALL_DEALLOC = 0x44;

// Instruction flags
// clang-format off
constexpr uint32_t INSTR_FLAG_TYPE_I8   = 0b00000000000000000000000000000001;
constexpr uint32_t INSTR_FLAG_TYPE_I16  = 0b00000000000000000000000000000010;
constexpr uint32_t INSTR_FLAG_TYPE_I32  = 0b00000000000000000000000000000100;
constexpr uint32_t INSTR_FLAG_TYPE_I64  = 0b00000000000000000000000000001000;
constexpr uint32_t INSTR_FLAG_TYPE_F32  = 0b00000000000000000000000000010000;
constexpr uint32_t INSTR_FLAG_TYPE_F64  = 0b00000000000000000000000000100000;
constexpr uint32_t INSTR_FLAG_OP_ADD    = 0b00000000000000000000000001000000;
constexpr uint32_t INSTR_FLAG_OP_SUB    = 0b00000000000000000000000010000000;
constexpr uint32_t INSTR_FLAG_OP_MUL    = 0b00000000000000000000000100000000;
constexpr uint32_t INSTR_FLAG_OP_DIV    = 0b00000000000000000000001000000000;
constexpr uint32_t INSTR_FLAG_OP_MULS   = 0b00000000000000000000010000000000;
constexpr uint32_t INSTR_FLAG_OP_DIVS   = 0b00000000000000000000100000000000;
constexpr uint32_t INSTR_FLAG_OP_AND    = 0b00000000000000000001000000000000;
constexpr uint32_t INSTR_FLAG_OP_OR     = 0b00000000000000000010000000000000;
constexpr uint32_t INSTR_FLAG_OP_XOR    = 0b00000000000000000100000000000000;
constexpr uint32_t INSTR_FLAG_OP_NOT    = 0b00000000000000001000000000000000;
constexpr uint32_t INSTR_FLAG_OP_LSH    = 0b00000000000000010000000000000000;
constexpr uint32_t INSTR_FLAG_OP_RSH    = 0b00000000000000100000000000000000;
constexpr uint32_t INSTR_FLAG_OP_SRSH   = 0b00000000000001000000000000000000;
// Instruction masks
constexpr uint32_t INSTR_FLAG_TYPE_MASK = 0b00000000000001111111111111000000;
// clang-format on

enum class JumpCondition {
    UNCONDITIONAL,
    IF_EQUALS,
    IF_NOT_EQUALS,
    IF_GREATER_THAN,
    IF_LESS_THAN,
    IF_GREATER_EQUALS,
    IF_LESS_EQUALS,
};

// Note: For readability use snake_case for instruction function names
#define MAKE_INSTR(name)                                                       \
    uint32_t instr_##name(UVM* vm, uint32_t width, uint32_t flag)

// TODO: *_itype_ireg_ireg vs *_ireg_ireg
// Arithmetic
MAKE_INSTR(arithm_common_ireg_ireg);
MAKE_INSTR(arithm_common_freg_freg);
MAKE_INSTR(arithm_common_ireg_int);
MAKE_INSTR(arithm_common_freg_float);
MAKE_INSTR(bitwise_common_itype_ireg_ireg);
MAKE_INSTR(shift_common_ireg_ireg);
MAKE_INSTR(not_itype_ireg);
MAKE_INSTR(sqrt);
MAKE_INSTR(mod);
MAKE_INSTR(unsigned_cast_to_long);
MAKE_INSTR(signed_cast_to_long);
MAKE_INSTR(f2d);
MAKE_INSTR(d2f);
MAKE_INSTR(i2f);
MAKE_INSTR(i2d);
MAKE_INSTR(f2i);
MAKE_INSTR(d2i);
// Branching
MAKE_INSTR(cmp_ireg_ireg);
MAKE_INSTR(jmp);
// Function
MAKE_INSTR(call);
MAKE_INSTR(ret);
// Memory manip
MAKE_INSTR(push_int);
MAKE_INSTR(push_ireg);
MAKE_INSTR(pop);
MAKE_INSTR(pop_ireg);
MAKE_INSTR(load_int_ireg);
MAKE_INSTR(load_ro_ireg);
MAKE_INSTR(loadf_float_freg);
MAKE_INSTR(loadf_ro_freg);
MAKE_INSTR(store_ireg_ro);
MAKE_INSTR(storef_freg_ro);
MAKE_INSTR(copy_int_ro);
MAKE_INSTR(copy_ireg_ireg);
MAKE_INSTR(copy_ro_ro);
MAKE_INSTR(copyf_float_ro);
MAKE_INSTR(copyf_freg_freg);
MAKE_INSTR(copyf_ro_ro);
MAKE_INSTR(lea_ro_ireg);
// Syscall
MAKE_INSTR(syscall);
