set(TEST "libc-test")

add_executable(${TEST}
    test.c
)

target_link_libraries(${TEST} PRIVATE
    dlibc
    dandelion_file_system
    dandelion_runtime
    runtime
)
