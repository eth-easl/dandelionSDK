# Python Runtime

This folder contains all the necessary code to create a function that can act as a Pyhton runtime.
This function is based on the cpython interpreter.

## Building
To build the Python function, use the CMakeLists.txt in this folder.
The build process requires a working libc as well as a C compiler to use it.
By default, this libc and C compiler is pulled from the CI, but can also be configured to use a local version.
We also have build configuration for Python 3.12, 3.13 and 3.14, where 3.14 is the default.

To configure the build, there are the following parameters:
- `CPYTHON_TAG` the tag to use for the checking out the cpython repository.
- `DANDELION_CLANG` to set the path to the compiler to use, if a local version of the dandelionSDK compiler should be used.
- `ARCHITECTURE` and `PLATFORM` no local compiler is provided, the architecture and platform can be set using these parameters. They default to the build system's architecture and `debug` respectively.

To build the default use:
```
mkdir ../build_cpython
cd ../build_cpython
cmake ../dandelionSDK/python/CMakeLists.txt
```

To use other configurations use (from the root of this repository):
```
mkdir ../build_cpython
cd ../build_cpython
cmake ../dandelionSDK/python/CMakeLists.txt -DCPYTHON_TAG=<tag> -DDANDELION_CLANG=<clang_path>
```
Or 
```
mkdir ../build_cpython
cd ../build_cpython
cmake ../dandelionSDK/python/CMakeLists.txt -DCPYTHON_TAG=<tag> -DARCHITECTURE=<arch> -DPLATFORM=<plat>
```