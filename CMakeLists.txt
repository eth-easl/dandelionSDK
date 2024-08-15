cmake_minimum_required(VERSION 3.20)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(dandelion-sdk)

option(FREESTANDING "toggles the include of memcpy, memset, memmove and memcmp" ON)
option(DANDELION_FS "toggles include of lowes level file system provided by dandelion" OFF)
option(DEBUG "toggles debug build" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SYSTEM_LIB "dandelion_system")
set(RUNTIME_LIB "dandelion_runtime")
set(DANDELION_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# if freestanding is on add the FREESTANDING definition to all compilation
if(FREESTANDING)
add_compile_definitions(FREESTANDING)
endif()

add_subdirectory(system)
add_subdirectory(runtime)
add_subdirectory(examples)

if(DANDELION_FS)
    set(FILE_SYSTEM_LIB "dandelion_file_system")
    add_subdirectory(file_system)
endif()