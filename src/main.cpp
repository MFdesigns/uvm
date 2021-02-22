// ======================================================================== //
// Copyright 2021 Michel Fäh
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

#include "debug/debugger.hpp"
#include "error.hpp"
#include "uvm.hpp"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>

void printCLIUsage() {
    std::cout
        << "usage: uvm <source file> [--debug-server]\n";
}

int main(int argc, char* argv[]) {
    // Check if minimal CLI arguments are provided
    if (argc < 2) {
        printCLIUsage();
        return -1;
    }

    // Check if UVM was started with debug server flag
    if (strcmp(argv[1], "--debug-server") == 0) {
        Debugger dbg;
        dbg.startSession();
        return 0;
    }

    // Check if target UX file exists
    char* sourcePath = argv[1];
    std::filesystem::path p{sourcePath};
    if (!std::filesystem::exists(p)) {
        std::cout << "Target file '" << p.string() << "' does not exist\n";
        return -1;
    }

    UVM vmInstance;
    vmInstance.setFilePath(p);
    size_t fileSize = 0;
    uint8_t* buffer = vmInstance.readSource(p, &fileSize);

    bool loadStatus = vmInstance.loadFile(buffer, fileSize);
    if (loadStatus != UVM_SUCCESS) {
        std::cerr << "Could not load file\n";
        return -1;
    }

    // Deallocate buffer because it has no use after loading the file sections
    if (buffer != nullptr) {
        delete[] buffer;
    }

    bool initSuccess = vmInstance.init();
    if (!initSuccess) {
        std::cerr << "Could not initialize the virtual machine\n";
        return -1;
    }

    uint8_t status = vmInstance.run();
    if (status != UVM_SUCCESS) {
        std::cerr << "[RUNTIME ERROR] " << translateError(status)
                  << "\nVM exited with an error\n";
        return -1;
    }
}
