add_subdirectory(instr)

add_executable(uvm
    main.cpp
    uvm.cpp uvm.hpp
    memory.cpp memory.hpp
    debug/debugger.cpp debug/debugger.hpp
    debug/http.cpp debug/http.hpp
)

target_link_libraries(uvm PRIVATE instr)
