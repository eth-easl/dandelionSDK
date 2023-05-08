set(SOURCES cheri.c)
add_library(${SYSTEM_LIB} ${SOURCES})
target_include_directories(${SYSTEM_LIB} PRIVATE ${CMAKE_SOURCE_DIR}/include)
