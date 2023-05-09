project(dandelion-sdk)
cmake_minimum_required(VERSION 3.20)

set(LIBRARY "dandelion")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LINUX_SYSCALL_LIB "linux_syscall")
set(FREEBSD_SYSCALL_LIB "freebsd_syscall")
set(SYSTEM_LIB "system")
set(RUNTIME_LIB "runtime")

add_subdirectory(system)
add_subdirectory(runtime)

# # target for compartment library
# add_library(${LIBRARY} STATIC
#   src/dandelion.c
# )

# # export include directory
# target_include_directories(${LIBRARY} PUBLIC include)

#set_target_properties(${LIBRARY} PROPERTIES LINKER_LANGUAGE C)
#target_compile_options(${LIBRARY} PRIVATE ${NOCAP_COMPILE_FLAGS})
#target_link_options(${LIBRARY} PRIVATE ${NOCAP_LINK_FLAGS})
