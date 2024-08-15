add_library(${FILE_SYSTEM_LIB} STATIC 
    file_system.c
    fs_implementation.c
    paths.c
)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${FILE_SYSTEM_LIB} PRIVATE DEBUG)
    target_compile_options(${FILE_SYSTEM_LIB} PRIVATE -g)
endif()

if(DANDELION_PLATFORM MATCHES "test")
    target_compile_definitions(${FILE_SYSTEM_LIB} PRIVATE TEST)
endif()

target_include_directories(${FILE_SYSTEM_LIB} PUBLIC ${DANDELION_ROOT}/include
)
target_link_options(${FILE_SYSTEM_LIB} PUBLIC -nostdlib)
target_link_libraries(${FILE_SYSTEM_LIB} PRIVATE 
    ${RUNTIME_LIB}
)