#! /bin/bash

# need to remove stdinc++ from input, as it causes a redundancy warning with stdinc, which causes errors for programs with -Werror
ARGS="$@"
ARGS="${ARGS//"-nostdinc++"/}"

@CMAKE_C_COMPILER@ @NEWLIB_C_FLAGS@ $ARGS
