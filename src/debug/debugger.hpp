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

constexpr uint64_t REQ_MAGIC = 0x3f697a65bcc37247;
constexpr uint64_t RES_MAGIC = 0x4772C3BC657A6921;
constexpr uint8_t RES_CLOSE = 0xDC;
constexpr uint8_t RES_HANDSHAKE = 0xD0;
constexpr uint8_t RES_NEXT_INSTR = 0xA0;
constexpr uint8_t RES_GET_REGS = 0xD2;

enum class DbgSessState {
    HANDSHAKE,
    RUNNING,
    CLOSED,
};

struct Debugger {
    HTTPServer Server;
    RequestParser Req;
    UVM VM;
    DbgSessState State = DbgSessState::HANDSHAKE;
    void startSession();
    void closeSession();
    bool handleHandshake(Response& res);
    bool handleRequest(Response& res);
    void appendRegisters(std::stringstream& stream);
};
