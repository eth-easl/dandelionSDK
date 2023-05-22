project(dandelion-sdk)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SYSTEM_LIB "dandelion_system")
set(RUNTIME_LIB "dandelion_runtime")
set(DANDELION_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(system)
add_subdirectory(runtime)
add_subdirectory(examples)
