/**
 * Copyright 2020 Michel Fäh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include "register.hpp"
#include "section.hpp"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

constexpr uint64_t UVM_START_ADDR = 0;
constexpr uint64_t UVM_STACK_SIZE = 4096;

struct HeaderInfo {
    uint8_t Version = 0;
    uint8_t Mode = 0;
    uint64_t StartAddress = 0;
};

class UVM {
  public:
    UVM(std::filesystem::path p);
    bool init();
    bool findMemSection(uint64_t vStartAddr,
                        uint32_t size,
                        MemSection** memSec) const;
    bool getMem(uint64_t vStartAddr,
                uint32_t size,
                uint8_t perms,
                uint8_t** ptr) const;
    bool memWrite(void* source, uint64_t vStartAddr, uint32_t size);
    bool run();

  private:
    std::filesystem::path SourcePath;
    uint32_t SourceBuffIndex = 0;
    std::unique_ptr<HeaderInfo> HInfo;
    std::unique_ptr<RegisterManager> RM;
    std::vector<MemSection> Sections;
    std::vector<MemBuffer> Buffers;
    void readSource();
};
