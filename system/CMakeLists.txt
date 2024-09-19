if(NOT DANDELION_PLATFORM)
set(DANDELION_PLATFORM "debug")
endif()

add_library(${SYSTEM_LIB} STATIC system.c)
if(FREESTANDING)
    target_sources(${SYSTEM_LIB} PRIVATE freestanding.c)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${SYSTEM_LIB} PRIVATE DEBUG)
    target_compile_options(${SYSTEM_LIB} PRIVATE -g)
endif()

target_include_directories(${SYSTEM_LIB} PRIVATE ${DANDELION_ROOT}/include)
target_compile_options(${SYSTEM_LIB} PRIVATE -fno-stack-protector)
target_link_options(${SYSTEM_LIB} PUBLIC -nostdlib)

install(TARGETS ${SYSTEM_LIB}
    ARCHIVE 
)

if(DANDELION_PLATFORM MATCHES "debug")
    add_subdirectory(platform/debug)
    add_subdirectory(syscall/linux)
elseif(DANDELION_PLATFORM MATCHES "test")
    add_subdirectory(platform/test)    
elseif(DANDELION_PLATFORM MATCHES "cheri")
    add_subdirectory(platform/cheri)
elseif(DANDELION_PLATFORM MATCHES "mmu_freebsd")
    add_subdirectory(platform/mmu_freebsd)
    add_subdirectory(syscall/freebsd)
elseif(DANDELION_PLATFORM MATCHES "mmu_linux")
    add_subdirectory(platform/mmu_linux)
    add_subdirectory(syscall/linux)
elseif(DANDELION_PLATFORM MATCHES "kvm")
    add_subdirectory(platform/kvm)
elseif(DANDELION_PLATFORM MATCHES "wasm")
    add_subdirectory(platform/wasm)
else()
    message(FATAL_ERROR "Invalid platform: ${DANDELION_PLATFORM}")
endif()
message(STATUS "Using platform: ${DANDELION_PLATFORM}")

