add_library(instr
    memory_manip.cpp memory_manip.hpp
    syscall.cpp syscall.hpp
    function.cpp function.hpp
    arithmetic.cpp arithmetic.hpp
    branching.cpp branching.hpp
)

set_target_properties(instr PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(instr PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
