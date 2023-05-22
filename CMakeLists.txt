set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(dandelion-sdk)
cmake_minimum_required(VERSION 3.20)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SYSTEM_LIB "dandelion_system")
set(RUNTIME_LIB "dandelion_runtime")

add_subdirectory(system)
add_subdirectory(runtime)
add_subdirectory(examples)
