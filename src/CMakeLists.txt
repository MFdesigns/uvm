add_subdirectory(instr)

add_executable(uvm
    main.cpp
    uvm.cpp uvm.hpp
    register.cpp register.hpp
    memory.cpp memory.hpp
)

target_link_libraries(uvm PRIVATE instr)
