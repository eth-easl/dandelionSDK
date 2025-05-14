# actions to make the SDK build possible for outside sources

if(DANDELION_PLATFORM MATCHES "wasm")
    set(TEMPLATE_TARGET "--target=wasm32-unknown-unknown")
else()
    set(TEMPLATE_TARGET "--target=${ARCHITECTURE}-unknown-elf")
    set(BUILTIN_LIB "\"\${CMAKE_CURRENT_SOURCE_DIR}/lib/generic/libclang_rt.builtins-${ARCHITECTURE}.a\"")
endif()

configure_file(CMakeTemplate.txt ${CMAKE_INSTALL_PREFIX}/CMakeLists.txt
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ
    @ONLY
)

if(NEWLIB)
    # install linker script needed to compile with newlib
    install(FILES linker.ld DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()