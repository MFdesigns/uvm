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

#include "section.hpp"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

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

  private:
    std::filesystem::path SourcePath;
    std::unique_ptr<MemBuffer> Source;
    std::unique_ptr<HeaderInfo> HInfo;
    std::vector<MemSection> Sections;
    std::vector<MemBuffer> Buffers;
    void readSource();
};
