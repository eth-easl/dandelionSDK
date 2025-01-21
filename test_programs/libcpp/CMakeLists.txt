set(TEST "libcpp-test")

add_executable(${TEST}
  test.cpp
)

target_link_libraries(${TEST} PRIVATE
  dlibcxx
  dlibc
  dandelion_runtime
)
