add_library(${LIBC_EXTENSION} STATIC)

target_sources(${LIBC_EXTENSION}
    PRIVATE
    dirent.c
    dlfcn.c
    netdb.c
    socket.c
    uio.c
    PUBLIC FILE_SET public_headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/
    FILES
    include/dlfcn.h
    include/netdb.h
    include/sys/cpuset.h
    include/sys/dirent.h
    include/sys/sched.h
    include/sys/socket.h
    include/sys/statvfs.h
    include/sys/uio.h
)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${LIBC_EXTENSION} PRIVATE DEBUG)
    target_compile_options(${LIBC_EXTENSION} PRIVATE -g)
endif()

target_link_libraries(${LIBC_EXTENSION} PRIVATE dlibc)

include(GNUInstallDirs)
install(TARGETS ${LIBC_EXTENSION}
    ARCHIVE FILE_SET public_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/
)

# always install after building, so the headers are available for libcxx
add_custom_command(
    TARGET ${LIBC_EXTENSION}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_CURRENT_BINARY_DIR}
)