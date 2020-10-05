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

#include <cstdint>
#include <memory>

enum class SectionType {
    NAME_STRING = 0x1,
    META_DATA = 0x2,
    DEBUG = 0x3,
    STATIC = 0x4,
    CODE = 0x5,
};

struct MemPermission {
    bool Write = false;
    bool Read = false;
    bool Execute = false;
};

struct MemSection {
    MemSection(uint64_t startAddress,
               uint64_t size,
               uint64_t nameAddress,
               SectionType type,
               std::unique_ptr<MemPermission> perms);
    uint64_t VStartAddress = 0;
    uint64_t Size = 0;
    uint64_t VNameAddress = 0;
    SectionType Type;
    std::unique_ptr<MemPermission> Perms;
};

struct MemBuffer {
    uint64_t Size;
    std::unique_ptr<uint8_t> Buffer;
};
