add_library(${RUNTIME_LIB} STATIC runtime.c)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${RUNTIME_LIB} PRIVATE DEBUG)
    target_compile_options(${RUNTIME_LIB} PRIVATE -g)
endif()

target_include_directories(${RUNTIME_LIB} PUBLIC ${DANDELION_ROOT}/include)
target_link_options(${RUNTIME_LIB} PUBLIC -nostdlib)
target_link_libraries(${RUNTIME_LIB} PRIVATE ${SYSTEM_LIB})