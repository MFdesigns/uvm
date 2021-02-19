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

#pragma once
#include "../uvm.hpp"
#include "http.hpp"
#include <cstdint>
#include <memory>
#include <vector>

constexpr uint64_t REQ_MAGIC = 0x3f697a65bcc37247;
constexpr uint64_t RES_MAGIC = 0x4772C3BC657A6921;

// Operation codes
constexpr uint8_t DBG_OPEN_DBG_SESS = 0x01;
constexpr uint8_t DBG_CLOSE_DBG_SESS = 0x02;
constexpr uint8_t DBG_SET_BREAKPNT = 0xB0;
constexpr uint8_t DBG_REMOVE_BREAKPNT = 0xB1;
constexpr uint8_t DBG_RUN_APP = 0xE0;
constexpr uint8_t DBG_NEXT_INSTR = 0xE1;
constexpr uint8_t DBG_CONTINUE_ = 0xE2;
constexpr uint8_t DBG_STOP_EXE = 0xE3;
constexpr uint8_t DBG_GET_REGS = 0x10;
constexpr uint8_t DBG_ERROR = 0xEE;
constexpr uint8_t DBG_EXE_FIN = 0xFF;

// Error codes
constexpr uint8_t ERR_ALREADY_IN_DEBUG_SESSION = 0x1;
constexpr uint8_t ERR_NOT_IN_DEBUG_SESSION = 0x2;
constexpr uint8_t ERR_RUNTIME_ERROR = 0x3;
constexpr uint8_t ERR_FILE_FORMAT_ERROR = 0x4;
constexpr uint8_t ERR_BREAKPOINT_ALREADY_SET = 0x5;
constexpr uint8_t ERR_BREAKPOINT_NOT_EXISTING = 0x6;

enum class DbgSessState {
    OPEN,
    RUNNING,
    CLOSED,
};

struct Debugger {
    HTTPServer Server;
    RequestParser Req;
    std::unique_ptr<UVM> VM;
    DbgSessState State = DbgSessState::OPEN;
    std::vector<uint64_t> Breakpoints;
    bool OnBreakpoint = false;
    void startSession();
    void closeSession();
    bool handleRequest(Response& res);
    void appendRegisters(std::stringstream& stream);
    void appendConsole(std::stringstream& stream);
    uint8_t continueToBreakpoint();
};
