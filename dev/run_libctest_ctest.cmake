if(NOT DEFINED SDK_SRC OR NOT DEFINED SDK_BUILD OR NOT DEFINED SDK_INSTALL)
  message(FATAL_ERROR "SDK_SRC, SDK_BUILD, and SDK_INSTALL must be defined")
endif()

if(NOT DEFINED LIBCTEST_DIR OR NOT DEFINED LIBCTEST_WORKTREE)
  message(FATAL_ERROR "LIBCTEST_DIR and LIBCTEST_WORKTREE must be defined")
endif()

if(NOT DEFINED PYTHON3_EXECUTABLE OR NOT DEFINED LIBCTEST_MAKE)
  message(FATAL_ERROR "PYTHON3_EXECUTABLE and LIBCTEST_MAKE must be defined")
endif()

if(NOT DEFINED SDK_ARCHITECTURE)
  message(FATAL_ERROR "SDK_ARCHITECTURE must be defined")
endif()

set(LOG_DIR "${LIBCTEST_DIR}/logs")
set(LOG_PATH "${LOG_DIR}/ctest.log")
set(CSV_PATH "${LOG_DIR}/ctest.csv")
set(STDIO_DIR "${LOG_DIR}/test_stdio")
set(SDK_CC "${SDK_INSTALL}/${SDK_ARCHITECTURE}-unknown-dandelion-clang")
set(MAKE_STDOUT_LOG "${LIBCTEST_WORKTREE}/.ctest-make.stdout.log")
set(MAKE_STDERR_LOG "${LIBCTEST_WORKTREE}/.ctest-make.stderr.log")

if(NOT EXISTS "${SDK_BUILD}/CMakeCache.txt")
  message(FATAL_ERROR "SDK build directory is not configured: ${SDK_BUILD}")
endif()

file(MAKE_DIRECTORY "${LOG_DIR}")
file(WRITE "${LOG_PATH}" "")
file(APPEND "${LOG_PATH}" "Running libc-test through CMake orchestration\n")

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${SDK_BUILD}"
  RESULT_VARIABLE install_result
  COMMAND_ECHO STDOUT
)
if(NOT install_result EQUAL 0)
  message(FATAL_ERROR "cmake --install failed with exit code ${install_result}")
endif()

if(NOT EXISTS "${SDK_INSTALL}/create-compiler.sh")
  message(FATAL_ERROR "Missing compiler wrapper generator: ${SDK_INSTALL}/create-compiler.sh")
endif()

execute_process(
  COMMAND "${SDK_INSTALL}/create-compiler.sh" -c clang-20
  RESULT_VARIABLE wrapper_result
  COMMAND_ECHO STDOUT
)
if(NOT wrapper_result EQUAL 0)
  message(FATAL_ERROR "create-compiler.sh failed with exit code ${wrapper_result}")
endif()

if(EXISTS "${SDK_CC}")
  file(CHMOD "${SDK_CC}" FILE_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
  )
endif()

if(NOT EXISTS "${SDK_CC}")
  message(FATAL_ERROR "Expected compiler wrapper not found: ${SDK_CC}")
endif()

execute_process(
  COMMAND "${PYTHON3_EXECUTABLE}" "${SDK_SRC}/dev/dev_prepare_libctest.py" --fetch-missing
  RESULT_VARIABLE prepare_result
  COMMAND_ECHO STDOUT
)
if(NOT prepare_result EQUAL 0)
  message(FATAL_ERROR "dev_prepare_libctest.py failed with exit code ${prepare_result}")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
          "${LIBCTEST_WORKTREE}/config.mak.dlibc"
          "${LIBCTEST_WORKTREE}/config.mak"
)
file(MAKE_DIRECTORY "${LIBCTEST_WORKTREE}/input_sets")
file(MAKE_DIRECTORY "${LIBCTEST_WORKTREE}/output_sets")
file(MAKE_DIRECTORY "${STDIO_DIR}")

message(STATUS "Building and running libc-test")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "DLIBC_STDIO_ARTIFACTS=${STDIO_DIR}" "${LIBCTEST_MAKE}" -k "-j${CMAKE_BUILD_PARALLEL_LEVEL}" "CC=${SDK_CC}"
  WORKING_DIRECTORY "${LIBCTEST_WORKTREE}"
  RESULT_VARIABLE make_result
  OUTPUT_FILE "${MAKE_STDOUT_LOG}"
  ERROR_FILE "${MAKE_STDERR_LOG}"
)
if(EXISTS "${MAKE_STDOUT_LOG}")
  file(READ "${MAKE_STDOUT_LOG}" MAKE_STDOUT_CONTENT)
  file(APPEND "${LOG_PATH}" "${MAKE_STDOUT_CONTENT}")
endif()
if(EXISTS "${MAKE_STDERR_LOG}")
  file(READ "${MAKE_STDERR_LOG}" MAKE_STDERR_CONTENT)
  file(APPEND "${LOG_PATH}" "${MAKE_STDERR_CONTENT}")
endif()

execute_process(
  COMMAND "${PYTHON3_EXECUTABLE}" "${LIBCTEST_WORKTREE}/gen_csv.py" "${LIBCTEST_WORKTREE}/src"
  OUTPUT_FILE "${CSV_PATH}"
  RESULT_VARIABLE csv_result
)
if(NOT csv_result EQUAL 0)
  message(FATAL_ERROR "gen_csv.py failed with exit code ${csv_result}")
endif()

execute_process(
  COMMAND "${PYTHON3_EXECUTABLE}" "${LIBCTEST_WORKTREE}/check_csv.py" "${CSV_PATH}"
  RESULT_VARIABLE check_result
  COMMAND_ECHO STDOUT
)
if(NOT make_result EQUAL 0)
  message(STATUS "libc-test make returned exit code ${make_result}; using CSV summary as final result")
  file(APPEND "${LOG_PATH}" "\nlibc-test make returned exit code ${make_result}; using CSV summary as final result\n")
endif()
if(NOT check_result EQUAL 0)
  message(FATAL_ERROR "libc-test reported failures; see ${CSV_PATH}")
endif()
