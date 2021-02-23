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
#include "memory.hpp"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <sstream>
#include <vector>

struct HeaderInfo {
    uint8_t Version = 0;
    uint8_t Mode = 0;
    uint64_t StartAddress = 0;
};

enum class ExecutionMode {
    USER,
    DEBUGGER,
};

class UVM {
  public:
    /** Execution mode */
    ExecutionMode Mode = ExecutionMode::USER;
    /** Memory manager */
    MemManager MMU;
    /** Current opcode */
    uint8_t Opcode = 0;
    /** Console buffer used for the debugger */
    std::stringstream DbgConsole;

    void setFilePath(std::filesystem::path p);
    bool init();
    uint32_t run();
    uint32_t nextInstr();
    uint8_t* readSource(std::filesystem::path p, size_t* size);
    uint32_t loadFile(uint8_t* buff, size_t size);

  private:
    /** Source file path */
    std::filesystem::path SourcePath;
    /** Header information */
    HeaderInfo HInfo;
};

bool validateHeader(HeaderInfo* info, uint8_t* source, size_t size);
