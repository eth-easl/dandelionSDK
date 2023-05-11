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
