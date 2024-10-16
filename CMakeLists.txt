cmake_minimum_required(VERSION 3.23)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(dandelion-sdk)

include(ExternalProject)

# binary options
option(FREESTANDING "toggles the include of memcpy, memset, memmove and memcmp" ON)
option(DANDELION_FS "toggles include of lowes level file system provided by dandelion" OFF)
option(NEWLIB "toggles builing of newlib on top of dandelion interface" OFF)

# string options
set(ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "the architecture to build for")
set(DANDELION_PLATFORM "debug" CACHE STRING "backend to build for")
message(STATUS "Building for ${DANDELION_PLATFORM} on ${ARCHITECTURE}")

# Constants
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SYSTEM_LIB "dandelion_system")
set(RUNTIME_LIB "dandelion_runtime")
set(DANDELION_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "${CMAKE_BINARY_DIR}/dandelion_sdk")
endif()

# find compiler runtime
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -print-file-name=include
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE COMPILER_RUNTIME_INCLUDE
)

# set global compile options
add_compile_options(--target=${ARCHITECTURE}-none-elf -nostdinc -idirafter${COMPILER_RUNTIME_INCLUDE})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(WARNING "CMAKE_BUILD_TYPE not explicitly set, setting to Release")
endif()
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_options(-gdwarf-4)
endif()

# set dependent options
if(NEWLIB)
    set(DANDELION_FS ON)
endif()



# if freestanding is on add the FREESTANDING definition to all compilation
if(FREESTANDING)
add_compile_definitions(FREESTANDING)
endif()

add_subdirectory(system)
add_subdirectory(runtime)
add_subdirectory(test_programs)

if(DANDELION_FS)
    set(FILE_SYSTEM_LIB "dandelion_file_system")
    add_subdirectory(file_system)
endif()

# set default level of parallelism
if(NOT CMAKE_BUILD_PARALLEL_LEVEL)
    cmake_host_system_information(RESULT CMAKE_BUILD_PARALLEL_LEVEL QUERY NUMBER_OF_LOGICAL_CORES)
else()
    message(WARNING "build parallel was already set")
endif()

if(NEWLIB) 
    set(NEWLIB_C_FLAGS "-nostdinc")
    string(APPEND NEWLIB_C_FLAGS " -nostdlib")
    string(APPEND NEWLIB_C_FLAGS " -idirafter${COMPILER_RUNTIME_INCLUDE}")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
      string(APPEND NEWLIB_C_FLAGS " -gdwarf-4")
    endif()

    set(NEWLIB_COMPILER ${CMAKE_BINARY_DIR}/compiler_wrapped.sh)
    configure_file(newlib_shim/compiler_wrapper_template.sh ${NEWLIB_COMPILER} 
        FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE)

    set(NEWLIB_CONFIG_ARGS "--host=${ARCHITECTURE}-dandelion")
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
        COMMAND CC=${NEWLIB_COMPILER} <SOURCE_DIR>/newlib/configure --prefix=<INSTALL_DIR> ${NEWLIB_CONFIG_ARGS}
        BUILD_COMMAND CC=${NEWLIB_COMPILER} make -j ${CMAKE_BUILD_PARALLEL_LEVEL}
        INSTALL_COMMAND make install
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                <INSTALL_DIR>/${ARCHITECTURE}-dandelion/include
                ${CMAKE_INSTALL_PREFIX}/include
         COMMAND ${CMAKE_COMMAND} -E copy_directory
                <INSTALL_DIR>/${ARCHITECTURE}-dandelion/lib
                ${CMAKE_INSTALL_PREFIX}/lib
    )

    add_library(dlibc INTERFACE IMPORTED GLOBAL)
    add_dependencies(dlibc newlib)
    target_include_directories(dlibc INTERFACE "${CMAKE_INSTALL_PREFIX}/include") 
    target_link_libraries(dlibc INTERFACE 
        ${RUNTIME_LIB}
        ${FILE_SYSTEM_LIB}
        ${CMAKE_INSTALL_PREFIX}/lib/libm.a
        ${CMAKE_INSTALL_PREFIX}/lib/libg.a
        ${CMAKE_INSTALL_PREFIX}/lib/libc.a
    )
    target_link_options(dlibc INTERFACE -nostdlib)

    set(LLVM_C_FLAGS "-D_GNU_SOURCE=1")
    string(APPEND LLVM_C_FLAGS " -D_POSIX_TIMERS")
    string(APPEND LLVM_C_FLAGS " -D__ELF__")
    string(APPEND LLVM_C_FLAGS " -D__DANDELION__")
    string(APPEND LLVM_C_FLAGS " --sysroot=${CMAKE_INSTALL_PREFIX}")
    string(APPEND LLVM_C_FLAGS " -isystem ${CMAKE_INSTALL_PREFIX}/include")
    string(APPEND LLVM_C_FLAGS " -idirafter${COMPILER_RUNTIME_INCLUDE}")
    string(APPEND LLVM_C_FLAGS " -nostdlib")
    string(APPEND LLVM_C_FLAGS " -nostdinc")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        string(APPEND LLVM_C_FLAGS " -gdwarf-4")
    endif()

    set(LLVM_CXX_FLAGS "-D_GNU_SOURCE=1")
    string(APPEND LLVM_CXX_FLAGS " -D_POSIX_TIMERS")
    string(APPEND LLVM_CXX_FLAGS " -D__ELF__")
    # need __STRICT_ANSI__ to be undefined, for alloca to be defined, which is not strict ansi but expected by libcxx
    string(APPEND LLVM_CXX_FLAGS " -U__STRICT_ANSI__")
    string(APPEND LLVM_CXX_FLAGS " -D__DANDELION__")
    string(APPEND LLVM_CXX_FLAGS " --sysroot=${CMAKE_INSTALL_PREFIX}")
    string(APPEND LLVM_CXX_FLAGS " -isystem ${CMAKE_INSTALL_PREFIX}/include")
    string(APPEND LLVM_CXX_FLAGS " -idirafter${COMPILER_RUNTIME_INCLUDE}")
    string(APPEND LLVM_CXX_FLAGS " -nostdlib")
    string(APPEND LLVM_CXX_FLAGS " -nostdinc")
    string(APPEND LLVM_CXX_FLAGS " -Wno-narrowing")
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        string(APPEND LLVM_CXX_FLAGS " -gdwarf-4")
    endif()

    ExternalProject_Add(
        llvmproject
        PREFIX llvmproject
        GIT_REPOSITORY https://github.com/llvm/llvm-project
        GIT_TAG 1422f1bf2b14b3e633a4803cc1796ebd389ccef0
        UPDATE_DISCONNECTED 1
        SOURCE_SUBDIR runtimes
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        LIST_SEPARATOR ","
        CMAKE_ARGS
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
            -DCMAKE_SYSTEM_NAME=Generic
            -DCMAKE_C_COMPILER_TARGET=${ARCHITECTURE}-none-elf
            -DCMAKE_CXX_COMPILER_TARGET=${ARCHITECTURE}-none-elf
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DLLVM_ENABLE_RUNTIMES=libunwind,libcxxabi,libcxx
            -DCMAKE_C_FLAGS:STRING=${LLVM_C_FLAGS}
            -DCMAKE_CXX_FLAGS:STRING=${LLVM_CXX_FLAGS}
            -DLLVM_INCLUDE_TESTS=OFF
            -DLIBUNWIND_ENABLE_SHARED=OFF
            -DLIBUNWIND_ENABLE_STATIC=ON
            -DLIBUNWIND_ENABLE_THREADS=OFF
            -DLIBUNWIND_IS_BAREMETAL=ON
            -DLIBCXXABI_BAREMETAL=ON
            -DLIBCXXABI_ENABLE_SHARED=OFF
            -DLIBCXXABI_ENABLE_THREADS=OFF
            -DLIBCXXABI_USE_LLVM_UNWINDER=ON
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

    add_library(dlibcxx INTERFACE IMPORTED GLOBAL)
    add_dependencies(dlibcxx llvmproject)
    target_include_directories(dlibcxx INTERFACE "${CMAKE_INSTALL_PREFIX}/include") 
    target_link_libraries(dlibcxx INTERFACE 
        dlibc
        ${CMAKE_INSTALL_PREFIX}/lib/libm.a
        ${CMAKE_INSTALL_PREFIX}/lib/libg.a
        ${CMAKE_INSTALL_PREFIX}/lib/libc.a
    )
    target_link_options(dlibcxx INTERFACE -nostdlib -D_POSIX_TIMERS -D_GNU_SOURCE=1 -T${CMAKE_CURRENT_SOURCE_DIR}/cxx_eh_frame.ld)

endif()
