#! /bin/bash
@CMAKE_C_COMPILER@ -nostdinc -nostdlib -gdwarf-4 -idirafter @COMPILER_RUNTIME_INCLUDE@ "$@"