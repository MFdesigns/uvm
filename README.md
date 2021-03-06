# Universal Virtual Machine

![UVM Logo](/res/UVM_logo_white.png)

UVM is a register-based process virtual machine which runs on Windows, macOS and Linux operating systems.

## Building UVM

### Requirements
- CMake
- MSVC / GCC / Clang

1. Getting the source code
   - `git clone https://github.com/MFdesigns/UVM.git`
2. Building
   - `cd UVM`
   - `mkdir build`
   - `cd build`
   - `cmake .. -DCMAKE_BUILD_TYPE=<build-type>`\
   Build types:
     - Debug
     - Release
    - `cmake --build .`
3. Output directory
   - On Windows: `UVM/build/<build-type>/uvm.exe`
   - On Linux/macOS: `UVM/build/uvm`

## Generating Documentation
This project uses Doxygen to generate documentation. To output HTML documentation execute the following command in the project folder:
 - `doxygen .\Doxyfile`
