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

MemSection::MemSection(uint64_t startAddress,
                       uint64_t size,
                       uint64_t nameAddress,
                       SectionType type,
                       std::unique_ptr<MemPermission> perms)
    : VStartAddress(startAddress), Size(size), VNameAddress(nameAddress),
      Type(type), Perms(std::move(perms)) {}
