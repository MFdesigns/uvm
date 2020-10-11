add_subdirectory(instr)

add_executable(uvm
    main.cpp
    uvm.cpp uvm.hpp
    section.cpp section.hpp
    register.cpp register.hpp
)

target_link_libraries(uvm PRIVATE instr)
