add_executable(simple simple.c)
target_link_libraries(simple dandelion_runtime)
target_link_options(simple PRIVATE -nostdlib -static)
