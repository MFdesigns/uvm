# Universal Virtual Machine

![UVM Logo](/res/UVM_logo_white.png)

UVM is a register-based process virtual machine which runs on Windows, macOS and Linux operating systems.

## Building UVM

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
   - On Windows: `UVM/build/<build-type>/uvm.exe`\
   - On Linux/macOS: `UVM/build/uvm`
