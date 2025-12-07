cmake_minimum_required(VERSION 3.5)
project(PortOps C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)

option(BUILD_SHARED "Build shared library" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Library
set(SRC
    src/ops_generic.c
)

add_library(portops ${SRC})
target_include_directories(portops PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (BUILD_SHARED)
    set_target_properties(portops PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Example executable
if (BUILD_EXAMPLES)
    add_executable(ops_demo examples/main.c)
    target_link_libraries(ops_demo PRIVATE portops m)
    target_include_directories(ops_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

# Install
install(TARGETS portops
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
