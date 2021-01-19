set(SOURCE_FILES
    main.cpp
    uvm.cpp uvm.hpp
    memory.cpp memory.hpp
    debug/debugger.cpp debug/debugger.hpp
    debug/http.cpp debug/http.hpp
    instr/memory_manip.cpp instr/memory_manip.hpp
    instr/syscall.cpp instr/syscall.hpp
    instr/function.cpp instr/function.hpp
    instr/arithmetic.cpp instr/arithmetic.hpp
    instr/branching.cpp instr/branching.hpp
    )

# Win32 specific platform files
if(WIN32)
    set(PLATFORM_FILES
        platform/win32_http.cpp
    )
# Linux and MacOS shared platform files
elseif(UNIX)
    set(PLATFORM_FILES
        # unix files
    )
    # MacOS specific platform files
    if(APPLE)
        # Append additional macos files to unix
    endif()
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${PLATFORM_FILES})
