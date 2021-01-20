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
#include "uvm.hpp"
#include <cstring>
#include <filesystem>
#include <iostream>

void printCLIUsage() {
    std::cout << "usage: uvm <path>\n";
}

int main(int argc, char* argv[]) {
    // Check if minimal CLI arguments are provided
    if (argc < 2) {
        printCLIUsage();
        return -1;
    }

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
    vmInstance.readSource();
    bool initSuccess = vmInstance.init();
    if (!initSuccess) {
        return -1;
    }

    bool exit = vmInstance.run();
    if (!exit) {
        std::cout << "VM exited with an error\n";
        return -1;
    }
}
