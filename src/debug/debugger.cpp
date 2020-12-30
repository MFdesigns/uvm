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

#include "debugger.hpp"
#include "http.hpp"
#include <iostream>

void Debugger::startSession() {
    std::cout << "[DEBUGGER] Starting up debug server...\n";
    Server.startup();

    Response closeSessionRes;
    closeSessionRes.Code = ResponseCode::OK_200;
    closeSessionRes.Headers["Access-Control-Allow-Origin"] = "*";
    closeSessionRes.Body.write(reinterpret_cast<const char*>(&RES_MAGIC), 8);
    closeSessionRes.Body << RES_CLOSE;
    closeSessionRes.fillBuffer();

    while (State != DbgSessState::CLOSED) {
        Response res;
        res.Code = ResponseCode::OK_200;
        res.Headers["Access-Control-Allow-Origin"] = "*";
        res.Body.write(reinterpret_cast<const char*>(&RES_MAGIC), 8);

        std::cout << "[DEBUGGER] Listening for incoming requests...\n";
        Server.listenLoop(Req);

        switch (State) {
        case DbgSessState::HANDSHAKE: {
            if (handleHandshake(res)) {
                std::cout << "[DEBUGGER] Received successful handshake\n";
                State = DbgSessState::RUNNING;
            } else {
                // TODO: If handshake fails reset VM
                std::cout << "[DEBUGGER] Error: Expected handshake received "
                             "garbage\n";
            }
        } break;
        case DbgSessState::RUNNING: {
            if (!handleRequest(res)) {
                std::cout << "[DEBUGGER] Error: could not handle request\n";
            }
        } break;
        }

        if (State == DbgSessState::CLOSED) {
            Server.sendReq(closeSessionRes.Stream);
        } else {
            res.fillBuffer();
            Server.sendReq(res.Stream);
        }
        Req.reset();
        Server.shutdownSock();
    }

    closeSession();
}

void Debugger::closeSession() {
    std::cout << "[DEBUGGER] Closing debug server...\n";
    Server.close();
}

bool Debugger::handleHandshake(Response& res) {
    // Check for minimal content size
    constexpr size_t MIN_CONTENT_SIZE = 0xD;
    if (Req.ContentLength < MIN_CONTENT_SIZE) {
        res.Code = ResponseCode::BAD_REQUEST_400;
        return false;
    }

    uint8_t* buff = Req.Content;

    uint64_t buffMagic = *reinterpret_cast<uint64_t*>(buff);
    if (buffMagic != REQ_MAGIC) {
        res.Code = ResponseCode::BAD_REQUEST_400;
        return false;
    }

    uint8_t operation = buff[8];
    if (operation != RES_HANDSHAKE) {
        res.Code = ResponseCode::BAD_REQUEST_400;
        return false;
    }

    uint32_t pathStrSize = *reinterpret_cast<uint64_t*>(&buff[9]);
    std::string path{reinterpret_cast<char*>(&buff[13]), pathStrSize};

    uint32_t fileSize = *reinterpret_cast<uint32_t*>(&buff[13 + pathStrSize]);
    uint8_t* fileBuff = &buff[13 + pathStrSize + 4];

    VM.setFilePath(std::filesystem::path{path});
    VM.addSourceFromBuffer(fileBuff, fileSize);
    VM.Mode = ExecutionMode::DEBUGGER;

    if (!VM.init()) {
        res.Code = ResponseCode::BAD_REQUEST_400;
        return false;
    }

    res.Code = ResponseCode::OK_200;
    res.Body << RES_HANDSHAKE;

    return true;
}

bool Debugger::handleRequest(Response& res) {
    uint8_t* buff = Req.Content;

    uint64_t buffMagic = *reinterpret_cast<uint64_t*>(buff);
    if (buffMagic != REQ_MAGIC) {
        return false;
    }

    uint8_t operation = buff[8];
    switch (operation) {
    case RES_NEXT_INSTR:
        if (!VM.nextInstr()) {
            // Send error code
            return false;
        }

        if (VM.Opcode == OP_EXIT) {
            State = DbgSessState::CLOSED;
            return true;
        }

        res.Body << RES_NEXT_INSTR;
        appendRegisters(res.Body);
        appendConsole(res.Body);
        break;
    case RES_GET_REGS:
        res.Body << RES_GET_REGS;
        appendRegisters(res.Body);
        break;
    case RES_CLOSE:
        State = DbgSessState::CLOSED;
        break;
    case RES_CONTINUE:
        continueToBreakpoint();
        if (VM.Opcode == OP_EXIT) {
            State = DbgSessState::CLOSED;
            return true;
        }

        res.Body << RES_CONTINUE;
        appendRegisters(res.Body);
        appendConsole(res.Body);
        break;
    case RES_SET_BREAKPOINT: {
        // TODO: Range check parameter. What if breakpoint already exists?
        uint64_t breakpoint = *reinterpret_cast<uint64_t*>(&buff[9]);
        Breakpoints.push_back(breakpoint);
    } break;
    case RES_REMOVE_BREAKPOINT: {
        uint64_t breakpoint = *reinterpret_cast<uint64_t*>(&buff[9]);
        // Find index of breakpoint
        // TODO: What if breakpoint does not exist
        size_t index = 0;
        for (size_t i = 0; i < Breakpoints.size(); i++) {
            if (Breakpoints[i] == breakpoint) {
                index = i;
                break;
            }
        }

        Breakpoints.erase(Breakpoints.begin() + index);
    } break;
    default:
        return false;
        break;
    }

    return true;
}

void Debugger::appendRegisters(std::stringstream& stream) {
    // Instruction pointer
    stream << static_cast<char>(0x1);
    stream.write(reinterpret_cast<char*>(&VM.MMU.IP), 8);
    // Stack pointer
    stream << static_cast<char>(0x2);
    stream.write(reinterpret_cast<char*>(&VM.MMU.SP), 8);
    // Base pointer
    stream << static_cast<char>(0x3);
    stream.write(reinterpret_cast<char*>(&VM.MMU.BP), 8);
    // Flags
    stream << static_cast<char>(0x4);
    uint64_t Flags = 0;
    uint64_t Carry = static_cast<uint64_t>(VM.MMU.Flags.Carry) << 63;
    uint64_t Zero = static_cast<uint64_t>(VM.MMU.Flags.Zero) << 62;
    uint64_t Signed = static_cast<uint64_t>(VM.MMU.Flags.Signed) << 61;
    Flags |= Carry;
    Flags |= Zero;
    Flags |= Signed;
    stream.write(reinterpret_cast<char*>(&Flags), 8);

    uint8_t regId = 0x5;
    for (IntVal& val : VM.MMU.GP) {
        stream << regId;
        stream.write(reinterpret_cast<char*>(&val.I64), 8);
        regId++;
    }
    for (FloatVal& val : VM.MMU.FP) {
        stream << regId;
        stream.write(reinterpret_cast<char*>(&val.F64), 8);
        regId++;
    }
}

void Debugger::appendConsole(std::stringstream& stream) {
    stream << VM.DbgConsole.rdbuf();
    // Clear console
    VM.DbgConsole.str(std::string());
    VM.DbgConsole.clear();
}

void Debugger::continueToBreakpoint() {
    while (VM.Opcode != OP_EXIT) {
        uint64_t ip = VM.MMU.IP;
        if (OnBreakpoint) {
            VM.nextInstr();
            OnBreakpoint = false;
        } else {
            bool isBreakpoint = false;
            for (uint32_t i = 0; i < Breakpoints.size(); i++) {
                if (Breakpoints[i] == ip) {
                    isBreakpoint = true;
                    break;
                }
            }

            if (isBreakpoint) {
                OnBreakpoint = true;
                return;
            } else {
                VM.nextInstr();
            }
        }
    }
}
