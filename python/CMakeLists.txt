cmake_minimum_required(VERSION 3.16)
project(CPythonBuild LANGUAGES C)

include(ExternalProject)
include(FetchContent)

# prevent in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source build detected")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# set default level of parallelism
if(NOT CMAKE_BUILD_PARALLEL_LEVEL)
    cmake_host_system_information(RESULT CMAKE_BUILD_PARALLEL_LEVEL QUERY NUMBER_OF_LOGICAL_CORES)
else()
    message(WARNING "build parallel was already set")
endif()

# Where CPython "make altinstall" will install
set(PYTHON_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Install prefix for CPython")

# Allow overwiting the tag from the CLI: -DCPYTHON_TAG=<tag/commit>
set(CPYTHON_TAG "3.14" CACHE STRING "CPython git tag/branch/commit")
set(ZLIB_TAG "v1.4" CACHE STRING "zlib git tag/branch/commit")
set(LIBFFI_TAG "v3.8.1" CACHE STRING "libffi git tag/branch/commit")
# check that there is a local python that can be used to bootstrap with the same version, required for freezing when cross compiling
find_program(BUILD_PYTHON3 NAMES python${CPYTHON_TAG} REQUIRED)
find_program(LIBTOOL NAMES libtool REQUIRED DOC "Required tool to build libffi")

# Allow overwiting the clang path from the CLI: -DDANDELION_CLANG=<path>
set(DANDELION_CLANG "" CACHE STRING "Path to clang to use for compilation")
set(DANDELION_CLANGPP "" CACHE STRING "Path to clang++ to use for compilation")
# If no overwrite is provided
if ("${DANDELION_CLANG}" STREQUAL "")
    # Additional arguments required to make sure we load the correct toolchain
    set(ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "the architecture to build for")
    set(PLATFORM "debug" CACHE STRING "backend to build for")
       
    # setup build toolchain
    FetchContent_Declare(
    dandelion_sdk
    DOWNLOAD_COMMAND gh release download latest -R eth-easl/dandelionFunctionInterface -p "*${CMAKE_BUILD_TYPE}_${PLATFORM}_${ARCHITECTURE}.tar.gz"
    COMMAND tar -xzf "dandelion_sdk_${CMAKE_BUILD_TYPE}_${PLATFORM}_${ARCHITECTURE}.tar.gz" -C ${CMAKE_BINARY_DIR}
    )
    FetchContent_MakeAvailable(dandelion_sdk)

    execute_process(
        COMMAND ${CMAKE_BINARY_DIR}/dandelion_sdk/create-compiler.sh -c "${CMAKE_C_COMPILER}"
    ) 
    set(DANDELION_CLANG ${CMAKE_BINARY_DIR}/dandelion_sdk/${ARCHITECTURE}-unknown-dandelion-clang)
    if ("${DANDELION_CLANGPP}" STREQUAL "")
        set(DANDELION_CLANGPP ${CMAKE_BINARY_DIR}/dandelion_sdk/${ARCHITECTURE}-unknown-dandelion-clang++)
    endif()
endif()
if ("${DANDELION_CLANGPP}" STREQUAL "")
    message(FATAL_ERROR "No set C++ compiler")
endif()

ExternalProject_Add(zlib
    PREFIX       "${CMAKE_BINARY_DIR}/zlib"
    SOURCE_DIR   "${CMAKE_BINARY_DIR}/zlib/src"
    BINARY_DIR   "${CMAKE_BINARY_DIR}/zlib/build"
    STAMP_DIR    "${CMAKE_BINARY_DIR}/zlib/stamp"
    TMP_DIR      "${CMAKE_BINARY_DIR}/zlib/tmp"
    INSTALL_DIR  "${CMAKE_BINARY_DIR}/python_deps"
    
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        "${ZLIB_TAG}"
    GIT_SHALLOW    1
    UPDATE_DISCONNECTED 1
    
    CONFIGURE_ENVIRONMENT_MODIFICATION
        CC=set:${DANDELION_CLANG}

    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DZLIB_BUILD_TESTING=OFF
        -DZLIB_BUILD_SHARED=OFF
        -DZLIB_BUILD_STATIC=ON
        -DZLIB_BUILD_MINIZIP=ON
        -DZLIB_INSTALL=ON

    # Helpful logging/TTY
    LOG_CONFIGURE 1
    LOG_BUILD     1
    LOG_INSTALL   1
    USES_TERMINAL_CONFIGURE 1
    USES_TERMINAL_BUILD     1
    USES_TERMINAL_INSTALL   1
)

ExternalProject_Add(libffi
    PREFIX       "${CMAKE_BINARY_DIR}/libffi"
    SOURCE_DIR   "${CMAKE_BINARY_DIR}/libffi/src"
    BINARY_DIR   "${CMAKE_BINARY_DIR}/libffi/build"
    STAMP_DIR    "${CMAKE_BINARY_DIR}/libffi/stamp"
    TMP_DIR      "${CMAKE_BINARY_DIR}/libffi/tmp"
    INSTALL_DIR  "${CMAKE_BINARY_DIR}/python_deps"

    GIT_REPOSITORY https://github.com/libffi/libffi.git
    GIT_TAG        "${LIBFFI_TAG}"
    GIT_SHALLOW    1
    UPDATE_DISCONNECTED 1

    # Configure step with environment variables
    CONFIGURE_COMMAND
    COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> <SOURCE_DIR>/autogen.sh
    COMMAND
        ${CMAKE_COMMAND} -E env
        CONFIG_SITE=${CMAKE_CURRENT_SOURCE_DIR}/config.site
        CC=${DANDELION_CLANG}
        CXX=${DANDELION_CLANGPP}
        CFLAGS=-g3\ -O2
        CXXFLAGS=-g3\ -O2
        HOST_CFLAGS=-Wno-unused-value\ -Wno-empty-body\ -Qunused-arguments
        LDFLAGS=-fuse-ld=lld
        LDSHARED=${DANDELION_CLANG}
        <SOURCE_DIR>/configure
        --host=x86_64-unknown-linux
        --build=x86_64-linux-gnu
        --disable-multi-os-directory
        --disable-exec_static-tramp
        --disable-docs
        --prefix=<INSTALL_DIR>

    # Helpful logging/TTY
    LOG_CONFIGURE 1
    LOG_BUILD     1
    LOG_INSTALL   1
    USES_TERMINAL_CONFIGURE 1
    USES_TERMINAL_BUILD     1
    USES_TERMINAL_INSTALL   1
)

set(FROZEN_MODULES_STRING
"'<asyncio.*>', '<collections.*>', '<concurrent.*>', '<ctypes.*>', '<curses.*>', '<dbm.*>', '<email.*>', '<html.*>', '<http.*>', '<idlelib.*>', '<json.*>', '<logging.*>', '<multiprocessing.*>', '<pydoc_data.*>', '<sqlite3.*>', '<tkinter.*>', '<tomllib.*>', '<turtledemo.*>', '<urllib.*>', '<venv.*>', '<wsgiref.*>', '<xml.*>', '<xmlrpc.*>', '<zipfile.*>', '<zoneinfo.*>', '__future__', '_aix_support', '_compat_pickle', '_markupbase', '_py_abc', '_pydatetime', '_pyio', '_pylong', '_strptime', '_threading_local', 'antigravity', 'argparse', 'ast', 'base64', 'bdb', 'bisect', 'bz2', 'cProfile', 'calendar', 'cmd', 'code', 'codeop', 'colorsys', 'compileall', 'configparser', 'contextlib', 'contextvars', 'copy', 'copyreg', 'csv', 'dataclasses', 'datetime', 'decimal', 'difflib', 'dis', 'enum', 'filecmp', 'fileinput', 'fnmatch', 'fractions', 'ftplib', 'functools', 'getopt', 'getpass', 'gettext', 'glob', 'graphlib', 'gzip', 'hashlib', 'heapq', 'hmac', 'imaplib', 'inspect', 'ipaddress', 'keyword', 'linecache', 'locale', 'lzma', 'mailbox', 'mimetypes', 'modulefinder', 'netrc', 'nturl2path', 'numbers', 'opcode', 'operator', 'optparse', 'pdb', 'pickle', 'pickletools', 'pkgutil', 'platform', 'plistlib', 'poplib', 'pprint', 'profile', 'pstats', 'pty', 'py_compile', 'pyclbr', 'pydoc', 'queue', 'quopri', 'random', 'reprlib', 'rlcompleter', 'sched', 'secrets', 'selectors', 'shelve', 'shlex', 'shutil', 'signal', 'smtplib', 'socket', 'socketserver', 'sre_compile', 'sre_constants', 'sre_parse', 'ssl', 'statistics', 'stringprep', 'struct', 'subprocess', 'symtable', 'tabnanny', 'tarfile', 'tempfile', 'textwrap', 'this', 'threading', 'timeit', 'token', 'tokenize', 'trace', 'traceback', 'tracemalloc', 'tty', 'turtle', 'types', 'typing', 'uuid', 'warnings',"
)

# To clean up the python build, go to the build directory and execute `make distclean`, this cleans up all configuration artifacts

ExternalProject_Add(cpython
    PREFIX       "${CMAKE_BINARY_DIR}/cpython"
    SOURCE_DIR   "${CMAKE_BINARY_DIR}/cpython/src"
    BINARY_DIR   "${CMAKE_BINARY_DIR}/cpython/build"
    STAMP_DIR    "${CMAKE_BINARY_DIR}/cpython/stamp"
    TMP_DIR      "${CMAKE_BINARY_DIR}/cpython/tmp"
    INSTALL_DIR  "${PYTHON_INSTALL_PREFIX}/cpython/target"

    GIT_REPOSITORY https://github.com/python/cpython.git
    GIT_TAG        "${CPYTHON_TAG}"
    GIT_SHALLOW    1
    UPDATE_DISCONNECTED 1

    # Configure step with environment variables
    CONFIGURE_COMMAND
    COMMAND ${CMAKE_COMMAND} -E echo "Updating freeze_modules.py with frozen modules"
    COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR>
        sed -i "s|#'<encodings.*>',|${FROZEN_MODULES_STRING}|g" Tools/build/freeze_modules.py
    COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> ${BUILD_PYTHON3} Tools/build/freeze_modules.py
    COMMAND
        ${CMAKE_COMMAND} -E env
        MODULE_MMAP_TRUE=#
        MODULE_BUILDTYPE=static
        CONFIG_SITE=${CMAKE_CURRENT_SOURCE_DIR}/config.site
        CC=${DANDELION_CLANG}
        CFLAGS=-g3\ -O2\ -D__GLIBC__\ -I${CMAKE_BINARY_DIR}/python_deps/include
        HOST_CFLAGS=-Wno-unused-value\ -Wno-empty-body\ -Qunused-arguments
        LDFLAGS=-fuse-ld=lld\ -L${CMAKE_BINARY_DIR}/python_deps/lib
        LDSHARED=${DANDELION_CLANG}
        CCSHARED=
        <SOURCE_DIR>/configure
        --host=x86_64-unknown-linux
        --build=x86_64-linux-gnu
        --disable-ipv6
        --disable-shared
        --prefix=<INSTALL_DIR>
        --with-build-python=${BUILD_PYTHON3}
        --without-builtin-hashlib-hashes
        --disable-test-modules
        --with-openssl=/opt/libssl
        --with-pydebug
    COMMAND make regen-frozen

    INSTALL_COMMAND cmake -E echo "Skipping install step."

    # Helpful logging/TTY
    LOG_CONFIGURE 1
    LOG_BUILD     1
    LOG_INSTALL   1
    USES_TERMINAL_CONFIGURE 1
    USES_TERMINAL_BUILD     1
    USES_TERMINAL_INSTALL   1
) 
ExternalProject_Add_Step(cpython sync_setup_local
    # Clean previous configs from source dir
    COMMAND ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git clean -dfX
    # Make sure Modules in build exists
    COMMAND ${CMAKE_COMMAND} -E make_directory "<BINARY_DIR>/Modules"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/Setup.local" "<BINARY_DIR>/Modules/Setup.local"
    # COMMAND "<BINARY_DIR>/config.status"
    # Remove anything that could contain leftover config in build
    COMMAND ${CMAKE_COMMAND} -E rm -f "<BINARY_DIR>/Modules/config.o"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "<BINARY_DIR>/python"
    COMMAND ${CMAKE_COMMAND} -E rm -f "<BINARY_DIR>/libpython*.a"
    DEPENDEES patch 
    DEPENDERS configure 
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Setup.local"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/config.site"
    WORKING_DIRECTORY "<BINARY_DIR>"
    COMMENT "Syncing Setup.local and clearing stale static archives"
)

# Make sure all the dependencies are ready before building cpython
add_dependencies(cpython zlib libffi)

# Build CPython as part of the default 'all' target
add_custom_target(cpython-all ALL DEPENDS cpython)
