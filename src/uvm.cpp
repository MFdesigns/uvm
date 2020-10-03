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

#include "uvm.hpp"
#include <fstream>

UVM::UVM(std::filesystem::path p): SourcePath(p) {
    readSource();
}

UVM::~UVM() {
    delete[] SourceBuffer;
}

void UVM::readSource() {
    // Read source file into buffer
    std::ifstream stream{SourcePath};

    // Get buffer size
    stream.seekg(0, std::ios::end);
    SourceSize = stream.tellg();
    stream.seekg(0, std::ios::beg);

    // Allocate new buffer of file size and read complete file to buffer
    SourceBuffer = new uint8_t[SourceSize];
    stream.read((char*)SourceBuffer, SourceSize);
}
