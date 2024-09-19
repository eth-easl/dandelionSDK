add_library(${FILE_SYSTEM_LIB} STATIC)

target_sources(${FILE_SYSTEM_LIB}
    PRIVATE
    file_system.c
    fs_implementation.c
    paths.c
    PUBLIC FILE_SET public_headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/fs_interface.h
)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${FILE_SYSTEM_LIB} PRIVATE DEBUG)
    target_compile_options(${FILE_SYSTEM_LIB} PRIVATE -g)
endif()

if(DANDELION_PLATFORM MATCHES "test")
    target_compile_definitions(${FILE_SYSTEM_LIB} PRIVATE TEST)
endif()

target_link_options(${FILE_SYSTEM_LIB} PUBLIC -nostdlib)
target_link_libraries(${FILE_SYSTEM_LIB} PRIVATE 
    ${RUNTIME_LIB}
)

include(GNUInstallDirs)
install(TARGETS ${FILE_SYSTEM_LIB}
    ARCHIVE
    FILE_SET public_headers
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dandelion
)