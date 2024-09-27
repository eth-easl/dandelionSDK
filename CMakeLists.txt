cmake_minimum_required(VERSION 3.23)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(dandelion-sdk)

include(ExternalProject)

# Options
option(FREESTANDING "toggles the include of memcpy, memset, memmove and memcmp" ON)
option(DANDELION_FS "toggles include of lowes level file system provided by dandelion" OFF)
option(NEWLIB "toggles builing of newlib on top of dandelion interface" OFF)

# Constants
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SYSTEM_LIB "dandelion_system")
set(RUNTIME_LIB "dandelion_runtime")
set(DANDELION_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "${CMAKE_BINARY_DIR}/dandelion_sdk")
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_options(-gdwarf-4)
endif()

# set dependent options
if(NEWLIB)
    set(DANDELION_FS ON)
endif()

execute_process(
    COMMAND ${CMAKE_C_COMPILER} -print-file-name=include
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE COMPILER_RUNTIME_INCLUDE
)

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


if(NEWLIB)
    if(NOT ARCHITECTURE)
        message( WARNING "No architecture set for libc, using system architecture ${CMAKE_SYSTEM_PROCESSOR}")
        set(ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
    endif()

    set(NEWLIB_CONFIG_ARGS "--host=${CMAKE_SYSTEM_PROCESSOR}-dandelion")
    list(APPEND NEWLIB_CONFIG_ARGS "--disable-newlib-reent-thread-local")
    list(APPEND NEWLIB_CONFIG_ARGS "--enable-newlib-io-pos-args")
    list(APPEND NEWLIB_CONFIG_ARGS "--enable-newlib-io-c99-formats")
    list(APPEND NEWLIB_CONFIG_ARGS "--enable-newlib-io-long-long")
    list(APPEND NEWLIB_CONFIG_ARGS "--enable-newlib-io-long-double")
    list(APPEND NEWLIB_CONFIG_ARGS "--disable-newlib-supplied-syscalls")
    list(APPEND NEWLIB_CONFIG_ARGS "--disable-newlib-multithread")
    # list(APPEND NEWLIB_CONFIG_ARGS "--enable-newlib-elix-level=2")

    ExternalProject_Add(
        newlib
        PREFIX newlib-cygwin
        GIT_REPOSITORY https://sourceware.org/git/newlib-cygwin.git
        GIT_TAG d8b21b8c77275941afcc5952fa3bb542413554ad #cygwin release 3.5.3
        # update disconnected to prevent pulling and rebuilding every time
        UPDATE_DISCONNECTED 1
        PATCH_COMMAND cd <SOURCE_DIR>
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/newlib_shim/patch_script <SOURCE_DIR> <BINARY_DIR>
        CONFIGURE_COMMAND cd <BINARY_DIR> 
        # official recoomendation is to build out of source dir with newlib and the option --with-newlib, but that build fails
        COMMAND <SOURCE_DIR>/newlib/configure --prefix=<INSTALL_DIR> ${NEWLIB_CONFIG_ARGS}
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                <INSTALL_DIR>/${CMAKE_SYSTEM_PROCESSOR}-dandelion/include
                ${CMAKE_INSTALL_PREFIX}/include
         COMMAND ${CMAKE_COMMAND} -E copy_directory
                <INSTALL_DIR>/${CMAKE_SYSTEM_PROCESSOR}-dandelion/lib
                ${CMAKE_INSTALL_PREFIX}/lib
    )
    ExternalProject_Get_Property(newlib INSTALL_DIR)
    set(NEWLIB_INSTALL_DIR "${INSTALL_DIR}/${CMAKE_SYSTEM_PROCESSOR}-dandelion")
    add_library(dlibc INTERFACE IMPORTED GLOBAL)
    add_dependencies(dlibc newlib)
    target_include_directories(dlibc INTERFACE "${NEWLIB_INSTALL_DIR}/include") 
    target_link_libraries(dlibc INTERFACE 
        ${RUNTIME_LIB}
        ${FILE_SYSTEM_LIB}
        ${NEWLIB_INSTALL_DIR}/lib/libm.a
        ${NEWLIB_INSTALL_DIR}/lib/libg.a
        ${NEWLIB_INSTALL_DIR}/lib/libc.a
    )
    target_link_options(dlibc INTERFACE -nostdlib)

    set(LLVM_C_FLAGS "-D_GNU_SOURCE=1")
    string(APPEND LLVM_C_FLAGS " -D_POSIX_TIMERS")
    string(APPEND LLVM_C_FLAGS " -D__DANDELION__")
    string(APPEND LLVM_C_FLAGS " --sysroot=${CMAKE_INSTALL_PREFIX}")
    string(APPEND LLVM_C_FLAGS " -isystem ${CMAKE_INSTALL_PREFIX}/include")
    string(APPEND LLVM_C_FLAGS " -idirafter${COMPILER_RUNTIME_INCLUDE}")
    string(APPEND LLVM_C_FLAGS " -nostdlib")
    string(APPEND LLVM_C_FLAGS " -nostdinc")

    set(LLVM_CXX_FLAGS "-D_GNU_SOURCE=1")
    string(APPEND LLVM_CXX_FLAGS " -D_POSIX_TIMERS")
    string(APPEND LLVM_CXX_FLAGS " -D__DANDELION__")
    string(APPEND LLVM_CXX_FLAGS " --sysroot=${CMAKE_INSTALL_PREFIX}")
    string(APPEND LLVM_CXX_FLAGS " -isystem ${CMAKE_INSTALL_PREFIX}/include")
    string(APPEND LLVM_CXX_FLAGS " -idirafter${COMPILER_RUNTIME_INCLUDE}")
    string(APPEND LLVM_CXX_FLAGS " -nostdlib")
    string(APPEND LLVM_CXX_FLAGS " -nostdinc")
    string(APPEND LLVM_CXX_FLAGS " -nostdinc++")
    string(APPEND LLVM_CXX_FLAGS " -Wno-narrowing")

    ExternalProject_Add(
        llvmproject
        PREFIX llvmproject
        GIT_REPOSITORY https://github.com/llvm/llvm-project
        GIT_TAG 1422f1bf2b14b3e633a4803cc1796ebd389ccef0
        SOURCE_SUBDIR runtimes
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        LIST_SEPARATOR ","
        CMAKE_ARGS
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
            -DCMAKE_SYSTEM_NAME=Generic
            -DCMAKE_C_COMPILER_TARGET=x86_64-none-elf
            -DCMAKE_CXX_COMPILER_TARGET=x86_64-none-elf
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DLLVM_ENABLE_RUNTIMES=libcxxabi,libcxx
            -DCMAKE_C_FLAGS:STRING=${LLVM_C_FLAGS}
            -DCMAKE_CXX_FLAGS:STRING=${LLVM_CXX_FLAGS}
            -DLLVM_INCLUDE_TESTS=OFF
            -DLIBCXXABI_BAREMETAL=ON
            -DLIBCXXABI_ENABLE_SHARED=OFF
            -DLIBCXXABI_ENABLE_THREADS=OFF
            -DLIBCXX_ENABLE_SHARED=OFF
            -DLIBCXX_ENABLE_STATIC=ON
            -DLIBCXX_INCLUDE_BENCHMARKS=OFF
            -DLIBCXX_CXX_ABI=libcxxabi
            -DLIBCXX_ENABLE_FILESYSTEM=ON
            -DLIBCXX_ENABLE_THREADS=OFF
            -DLIBCXX_ENABLE_MONOTONIC_CLOCK=OFF
            -DLIBCXX_ENABLE_RANDOM_DEVICE=OFF
    )
    add_dependencies(llvmproject newlib)

endif()
