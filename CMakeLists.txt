cmake_minimum_required(VERSION 3.17.1)
set(PROJECT_NAME uvm)

project(${PROJECT_NAME} VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CTEST_CONFIGURATION_TYPE )

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -march=native")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -O3 -march=native")
endif()

set(SOURCE_FILES
    src/uvm.cpp src/uvm.hpp
    src/memory.cpp src/memory.hpp
    src/error.hpp
    src/debug/debugger.cpp src/debug/debugger.hpp
    src/debug/http.cpp src/debug/http.hpp
    src/instr/instructions.hpp
    src/instr/memory_manip.cpp
    src/instr/syscall.cpp
    src/instr/function.cpp
    src/instr/arithmetic.cpp
    src/instr/branching.cpp
    )

# Win32 specific platform files
if(WIN32)
    set(PLATFORM_FILES
        src/platform/win32_http.cpp
    )
# Linux and MacOS shared platform files
elseif(UNIX)
    set(PLATFORM_FILES
        src/platform/linux_http.cpp
    )
    # MacOS specific platform files
    if(APPLE)
        # Append additional macos files to unix
    endif()
endif()

add_executable(${PROJECT_NAME} src/main.cpp ${SOURCE_FILES} ${PLATFORM_FILES})

include(CTest)

add_executable(test1 tests/test_main.cpp ${SOURCE_FILES} ${PLATFORM_FILES})
add_test(NAME test1 COMMAND test1)
