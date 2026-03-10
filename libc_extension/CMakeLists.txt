add_library(${LIBC_EXTENSION} STATIC)

target_sources(${LIBC_EXTENSION}
    PRIVATE
    arpa_inet.c
    aio.c
    dlfcn.c
    dirent.c
    mqueue.c
    nl_types.c
    poll.c
    semaphore.c
    sys_ipc.c
    sys_msg.c
    sys_sem.c
    monetary.c
    net_if.c
    netinet_in.c
    netdb.c
    pthread.c
    sched.c
    semaphore.c
    socket.c
    sys_mman.c
    sys_shm.c
    sys_utsname.c
    uio.c
    PUBLIC FILE_SET public_headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/
    FILES
    include/arpa/inet.h
    include/aio.h
    include/dlfcn.h
    include/mqueue.h
    include/nl_types.h
    include/poll.h
    include/semaphore.h
    include/monetary.h
    include/net/if.h
    include/netinet/in.h
    include/netinet/tcp.h
    include/netdb.h
    include/semaphore.h
    include/arpa/inet.h
    include/net/if.h
    include/netinet/in.h
    include/sys/cpuset.h
    include/sys/dirent.h
    include/sys/ipc.h
    include/sys/mman.h
    include/sys/msg.h
    include/sys/sem.h
    include/sys/shm.h
    include/sys/poll.h
    include/sys/sched.h
    include/sys/socket.h
    include/sys/statvfs.h
    include/sys/uio.h
    include/sys/un.h
    include/sys/utsname.h
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
