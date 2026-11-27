# Set defines only expected by C++
# _GNU_SOURCE is expected by the c++ standard library which is why we need to set it here.
-D_GNU_SOURCE


# C++ compiler flags
-isystem<CFGDIR>/include/c++/v1

# C++ linker flags
-lc++
-lc++abi
-lunwind

@@DANDELION_TARGET@-clang.cfg