add_library(${LINUX_SYSCALL_LIB} syscall_ret.c)
target_add_include_directories(${LINUX_SYSCALL_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)