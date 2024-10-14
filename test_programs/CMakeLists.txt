add_executable(simple simple.c)
target_compile_options(simple PRIVATE -fno-stack-protector)
target_link_libraries(simple dandelion_runtime)
target_link_options(simple PRIVATE --target=${ARCHITECTURE}-none-elf -nostdlib -static)
