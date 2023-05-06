project(dandelion-io)
cmake_minimum_required(VERSION ${CMAKE_VERSION})

set(LIBRARY "dandelion")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# target for compartment library
add_library(${LIBRARY} STATIC
  src/dandelion.c
)

# export include directory
target_include_directories(${LIBRARY} PUBLIC include)

#set_target_properties(${LIBRARY} PROPERTIES LINKER_LANGUAGE C)
#target_compile_options(${LIBRARY} PRIVATE ${NOCAP_COMPILE_FLAGS})
#target_link_options(${LIBRARY} PRIVATE ${NOCAP_LINK_FLAGS})
