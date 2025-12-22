set(TEST "libcpp-test")

add_executable(${TEST}
  test.cpp
)

target_link_libraries(${TEST} PRIVATE
  dlibcxx
  dlibc
  dandelion_file_system
  dandelion_runtime
  runtime
)

# prepare files to run the function in debug mode
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/input_sets)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/input_sets/in_a)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/input_1.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/input_sets/in_a)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/input_sets/in_b)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/input_2.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/input_sets/in_b)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output_sets)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output_sets/stdio)