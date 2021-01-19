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
#include <memory>

/**
 * Starts a new debug session and awaits incoming handshake
 */
void Debugger::startSession() {
    std::cout << "[DEBUGGER] Starting up debug server...\n";
    Server.startup();

    Response closeSessionRes;
    closeSessionRes.Code = ResponseCode::OK_200;
    closeSessionRes.Headers["Access-Control-Allow-Origin"] = "*";
    closeSessionRes.Body.write(reinterpret_cast<const char*>(&RES_MAGIC), 8);
    closeSessionRes.Body << DBG_CLOSE_DBG_SESS;
    closeSessionRes.fillBuffer();

    while (State != DbgSessState::CLOSED) {
        Response res;
        res.Code = ResponseCode::OK_200;
        res.Headers["Access-Control-Allow-Origin"] = "*";
        res.Body.write(reinterpret_cast<const char*>(&RES_MAGIC), 8);

        Server.listenLoop(Req);

        if (!handleRequest(res)) {
            std::cout << "[DEBUGGER] Error: could not handle request\n";
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

/**
 * Closes the current HTTP server
 */
void Debugger::closeSession() {
    std::cout << "[DEBUGGER] Closing debug server...\n";
    Server.close();
}

/**
 * Handles an incoming request
 * @param res Response which on valid request will hold the response data
 * @return On valid request returns true otherwise false
 */
bool Debugger::handleRequest(Response& res) {
    uint8_t* buff = Req.Content;

    uint64_t buffMagic = *reinterpret_cast<uint64_t*>(buff);
    if (buffMagic != REQ_MAGIC) {
        return false;
    }

    uint8_t operation = buff[8];
    switch (operation) {
    // If the server is already in a debugging session and we receive another
    // request for a debug session send appropriate error
    case DBG_OPEN_DBG_SESS: {
        if (State == DbgSessState::OPEN) {
            // Check for minimal content size
            constexpr size_t MIN_CONTENT_SIZE = 0x9;
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
            if (operation != DBG_OPEN_DBG_SESS) {
                res.Code = ResponseCode::BAD_REQUEST_400;
                return false;
            }

            res.Code = ResponseCode::OK_200;
            res.Body << DBG_OPEN_DBG_SESS;

            State = DbgSessState::RUNNING;

            return true;
        } else {
            res.Body << DBG_ERROR << ERR_ALREADY_IN_DEBUG_SESSION;
            return false;
        }
    } break;
    case DBG_CLOSE_DBG_SESS: {
        if (State == DbgSessState::RUNNING) {
            State = DbgSessState::CLOSED;
            return true;
        } else {
            res.Body << DBG_ERROR << ERR_NOT_IN_DEBUG_SESSION;
            return false;
        }
    }
    case DBG_RUN_APP: {
        if (State == DbgSessState::RUNNING) {
            // Delete previous UVM instances and create a new one
            VM.reset();
            VM = std::make_unique<UVM>();

            // Check if request meets minimal size to be valid before indexing
            // into it
            constexpr size_t MIN_VALID_REQ_SIZE = 13;
            if (Req.ContentLength < MIN_VALID_REQ_SIZE) {
                return false;
            }

            uint32_t fileSize = *reinterpret_cast<uint32_t*>(&buff[9]);
            uint8_t* fileBuff = &buff[13];

            // Check if given file size is valid
            if (Req.ContentLength < MIN_VALID_REQ_SIZE + fileSize) {
                return false;
            }

            VM->addSourceFromBuffer(fileBuff, fileSize);
            VM->Mode = ExecutionMode::DEBUGGER;

            if (!VM->init()) {
                res.Body << DBG_ERROR << ERR_FILE_FORMAT_ERROR;
                return false;
            }
        } else {
            res.Body << DBG_ERROR << ERR_NOT_IN_DEBUG_SESSION;
            return false;
        }

        // TODO: Runtime error?
        continueToBreakpoint();
        if (VM->Opcode == OP_EXIT) {
            res.Body << DBG_EXE_FIN;
            appendRegisters(res.Body);
            appendConsole(res.Body);
            VM.reset();
            OnBreakpoint = false;
        } else if (OnBreakpoint) {
            res.Body << DBG_RUN_APP;
            appendRegisters(res.Body);
            appendConsole(res.Body);
        }
    } break;
    case DBG_NEXT_INSTR: {
        // TODO: ERR_NO_UX_FILE and runtime error
        if (State == DbgSessState::RUNNING) {
            if (!VM->nextInstr()) {
                return false;
            }

            if (VM->Opcode == OP_EXIT) {
                res.Body << DBG_EXE_FIN;
                appendRegisters(res.Body);
                appendConsole(res.Body);
                VM.reset();
            } else {
                res.Body << DBG_NEXT_INSTR;
                appendRegisters(res.Body);
                appendConsole(res.Body);
            }
        } else {
            res.Body << DBG_ERROR << ERR_NOT_IN_DEBUG_SESSION;
        }
    } break;
    case DBG_SET_BREAKPNT: {
        // TODO: Range check parameter. What if breakpoint already exists?
        uint64_t breakpoint = *reinterpret_cast<uint64_t*>(&buff[9]);
        Breakpoints.push_back(breakpoint);
        res.Body << DBG_SET_BREAKPNT;
    } break;
    case DBG_REMOVE_BREAKPNT: {
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
        res.Body << DBG_REMOVE_BREAKPNT;
    } break;
    case DBG_CONTINUE_: {
        if (State != DbgSessState::RUNNING) {
            res.Body << DBG_ERROR << ERR_NOT_IN_DEBUG_SESSION;
            return false;
        }

        // TODO: Runtime error?
        continueToBreakpoint();
        if (VM->Opcode == OP_EXIT) {
            res.Body << DBG_EXE_FIN;
            appendRegisters(res.Body);
            appendConsole(res.Body);
            VM.reset();
        } else if (OnBreakpoint) {
            res.Body << DBG_CONTINUE_;
            appendRegisters(res.Body);
            appendConsole(res.Body);
        }
    } break;
    case DBG_STOP_EXE: {
        // TODO: Does UVM even run?
        res.Body << DBG_STOP_EXE;
        appendRegisters(res.Body);
        appendConsole(res.Body);
        VM.reset();
        OnBreakpoint = false;
    } break;
    default:
        return false;
        break;
    }

    return true;
}

/**
 * Appends register data to the given stream
 * @param stream Target stream
 */
void Debugger::appendRegisters(std::stringstream& stream) {
    // Instruction pointer
    stream << static_cast<char>(0x1);
    stream.write(reinterpret_cast<char*>(&VM->MMU.IP), 8);
    // Stack pointer
    stream << static_cast<char>(0x2);
    stream.write(reinterpret_cast<char*>(&VM->MMU.SP), 8);
    // Base pointer
    stream << static_cast<char>(0x3);
    stream.write(reinterpret_cast<char*>(&VM->MMU.BP), 8);
    // Flags
    stream << static_cast<char>(0x4);
    uint64_t Flags = 0;
    uint64_t Carry = static_cast<uint64_t>(VM->MMU.Flags.Carry) << 63;
    uint64_t Zero = static_cast<uint64_t>(VM->MMU.Flags.Zero) << 62;
    uint64_t Signed = static_cast<uint64_t>(VM->MMU.Flags.Signed) << 61;
    Flags |= Carry;
    Flags |= Zero;
    Flags |= Signed;
    stream.write(reinterpret_cast<char*>(&Flags), 8);

    uint8_t regId = 0x5;
    for (IntVal& val : VM->MMU.GP) {
        stream << regId;
        stream.write(reinterpret_cast<char*>(&val.I64), 8);
        regId++;
    }
    for (FloatVal& val : VM->MMU.FP) {
        stream << regId;
        stream.write(reinterpret_cast<char*>(&val.F64), 8);
        regId++;
    }
}

/**
 * Appends console output to given stream and flushes console
 * @param stream Target stream
 */
void Debugger::appendConsole(std::stringstream& stream) {
    stream << VM->DbgConsole.rdbuf();
    // Clear console
    VM->DbgConsole.str(std::string());
    VM->DbgConsole.clear();
}

/**
 * Executes virtual machine until next breakpoint is hit
 */
void Debugger::continueToBreakpoint() {
    while (VM->Opcode != OP_EXIT) {
        uint64_t ip = VM->MMU.IP;
        if (OnBreakpoint) {
            VM->nextInstr();
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
                VM->nextInstr();
            }
        }
    }
}
