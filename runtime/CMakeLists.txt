add_library(${RUNTIME_LIB} STATIC runtime.c)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${RUNTIME_LIB} PRIVATE DEBUG)
    target_compile_options(${RUNTIME_LIB} PRIVATE -g)
endif()

target_sources(${RUNTIME_LIB}
    PRIVATE runtime.c
    PUBLIC FILE_SET public_headers TYPE HEADERS
    BASE_DIRS ${DANDELION_ROOT}/include
    FILES
    ${DANDELION_ROOT}/include/dandelion/crt.h
    ${DANDELION_ROOT}/include/dandelion/io_buffer.h
    ${DANDELION_ROOT}/include/dandelion/runtime.h
    ${DANDELION_ROOT}/include/dandelion/system/system.h
)
target_compile_definitions(${RUNTIME_LIB} PRIVATE
    PAGE_SIZE=${PAGE_SIZE}
)

get_target_property(header_sets ${RUNTIME_LIB} HEADER_SET_public_headers)

target_link_options(${RUNTIME_LIB} PUBLIC -nostdlib)
target_link_libraries(${RUNTIME_LIB} PRIVATE ${SYSTEM_LIB})

install(TARGETS ${RUNTIME_LIB}
    ARCHIVE
    FILE_SET public_headers  
)