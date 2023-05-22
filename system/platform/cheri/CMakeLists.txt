set(SOURCES cheri.c)
target_sources(${SYSTEM_LIB} PRIVATE ${SOURCES})
target_compile_options(${SYSTEM_LIB} PUBLIC -mabi=aapc -march=morello)
target_link_options(${SYSTEM_LIB} PUBLIC -mabi=aapc -march=morello)
