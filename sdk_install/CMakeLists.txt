# actions to make the SDK build possible for outside sources

if(DANDELION_PLATFORM MATCHES "wasm")
    set(DANDELION_TARGET "wasm32-unknown-dandelion")
    set(TEMPLATE_TARGET "wasm32-unknown-unknown")
else()
    set(DANDELION_TARGET "${ARCHITECTURE}-unknown-dandelion")
    set(TEMPLATE_TARGET "${ARCHITECTURE}-unknown-elf")
    set(BUILTIN_LIB "\"\${CMAKE_CURRENT_SOURCE_DIR}/lib/generic/libclang_rt.builtins-${ARCHITECTURE}.a\"")
endif()

# Configure CMakeLists containing runtime, dlibc and dlibcxx
configure_file(CMakeTemplate.txt ${CMAKE_INSTALL_PREFIX}/CMakeLists.txt
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ
    @ONLY
)

if(NEWLIB)
    # Configure ARCHITECTURE-unknown-dandelion-clang
    configure_file(clang-template.cmake ${CMAKE_INSTALL_PREFIX}/${DANDELION_TARGET}-clang.cfg
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ
        @ONLY
    )

    # Configure ARCHITECTURE-unknown-dandelion-clang++
    configure_file(clangxx-template.cmake ${CMAKE_INSTALL_PREFIX}/${DANDELION_TARGET}-clang++.cfg
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ
        @ONLY
    )

    # Configure ARCHITECTURE-unknown-dandelion-clang++
    configure_file(toolchain-template.cmake ${CMAKE_INSTALL_PREFIX}/dandelion-toolchain.cmake
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ
        @ONLY
    )

    # Script to copy clang to the local folder
    configure_file(script-template.sh ${CMAKE_INSTALL_PREFIX}/create-compiler.sh
        FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ
        @ONLY
    )

    # install linker script needed to compile with newlib
    install(FILES linker.ld DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()