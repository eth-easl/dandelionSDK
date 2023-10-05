# Dandelion SDK
These libraries implement the necessary interface between a function and
the dandelion platform executing the function. It contains two libraries:
- `dandelion_system`, which is the lowest-level interface between the platform
and the function, defining data layouts and platform-specific functions.
- `dandelion_runtime`, which implements (slightly) higher-level primitives to
interact with inputs/outputs and heap memory. Dandelion libc is built on top
of this interface, and most user applications should be as well.

## Building
The target system is defined as an argument to cmake, i.e.
`cmake .. -DDANDELION_PLATFORM=<platform>`. Valid values for `<platform>` are
- `CHERI`, for the platform using cheri isolation
- `MMU_FREEBSD`, for the mmu-based platforms running on freebsd
- `MMU_LINUX`, for mmu-based platforms running on linux
- `DEBUG` (default), for running applications locally on a linux system with mock inputs,
dumping outputs to the terminal when terminating.

If no value is set, `DEBUG` will be used for the platform value.

## Examples
The examples are automatically built when compiling the library. 

## Freestanding
The GCC/Clang standard expects 4 functions to allways be provided in any environment (even freestanding), which allow the compiler to always just insert them.
(https://gcc.gnu.org/onlinedocs/gcc/Standards.html)
Because of this we also provide them.
They are:
- memcpy
- memset
- memmove
- memcmp

Currently they are just implemented somewhat sloppily, could optimize, have better QA or pull in a dependency if this becomes an issue