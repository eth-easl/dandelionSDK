cmake_minimum_required(VERSION 3.23)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(dandelion-sdk-tests)

set(SDK_PATH "" CACHE STRING "path to find sdk")

# add libraries as subdirectory, should also add alternative mode where we compile with cfg files
add_subdirectory("${SDK_PATH}/dandelion_sdk" "${CMAKE_BINARY_DIR}/dandelion_sdk_build")

add_subdirectory(libc)
add_subdirectory(libcpp)
