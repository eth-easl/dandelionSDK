# Dandelion SDK Overview
These libraries implement the necessary interface between a function and
the dandelion platform executing the function. It contains two libraries:
- `dandelion_system`, which is the lowest-level interface between the platform
and the function, defining data layouts and platform-specific functions.
- `dandelion_runtime`, which implements (slightly) higher-level primitives to
interact with inputs/outputs and heap memory. Dandelion libc is built on top
of this interface, and most user applications should be as well.

Additonally it also builds libraries on top of the above mentioned to support other languages.
These currently include:
- libc based on newlib
- libcxx based on the llvm version and the libc built above

A bigger selection of example functions can be found at [Function Examples](https://github.com/eth-easl/dandelionFunctionExamples)

## Platforms and Architectures

We support libraries for a variety of backends in Dandelion for both `x86_64` and `aarch64`
The platforms are:
- `CHERI`, for the platform using cheri isolation
- `MMU_FREEBSD`, for the mmu-based platforms running on freebsd
- `MMU_LINUX`, for mmu-based platforms running on linux
- `KVM`, for the kvm-based platform
- `WASM`, for the webassembly platform
- `DEBUG` (default), for running applications locally on a linux system with mock inputs,
dumping outputs to the terminal when terminating.

*Note:* There are currently issues with compiling the CXX library for aarch64.
Fixing it is on our agenda, but not of highest priority.
If you want to use Dandelion and need C++ support for aarch64, please get in touch and we may be able to provide experimental builds or bump the priority to get it fixed.

## Interface expectations
### libc
When using libc or any system on top of it values can be fed into stdin, argv and environ by specifying a input set called "stdio".
The system will look for items called "stdin", "argv" and "environ" in that set and automaticall make it available on the expected methods.
For argv and environ the content of the item is expected to be space separated as it would be on the command line.
To tell the programm it is called cat and has arguments foo and bar the total content of argv should be "cat foo bar" (without quotation marks).
Similarly for environ, if foo should be set to 1 and bar should be set to 3 the content of environ should be "foo=2 bar=3"
Currently there is no variable subsitution so defining foo to be 1 and then using foo in a later definition does not work.
Both the '' and "" can be used to escape strings that contain spaces.
Escaping the quote characters with backslashes is currently not supported, but they can be escaped by wrapping in the other type of quotes,
as everything within the outermost set of quotataion marks will be esacaped.
Example: `'"test"'` will become `"test"` 
Multiple quoted strings without a space in between are considered a single argument.
Example: `'test'"test"` will become `testtest` 

## Using

All libraries have prebuilt versions available for download or can be built locally.
The build process is descirbed bellow.
Either option will produce a folder `dandelion_sdk`.
This folder there are:
- `include` folder with the necessary header files
- `lib` folder with all linkable libraries
- `linker.ld` that is used when linking with the dlibc and dlibcxx
- `create-compiler.sh` a script that creates a local copy of the clang/clang++ compiler that automatically uses the configruration files provided. (Note if you move the compiler, the .cfg files need also to be moved, as clang looks in the folder the compiler is located at)
- `*.cfg` clang compiler config files
- `CMakeLists.txt` and `dandelion-toolchain.cmake` described in the CMake section bellow

### CMake

For CMake projects this can simply be added as a subfolder.
In this subfolder we define 3 target libraries:
- `dandelion_runtime`
- `dlibc`
- `dlibcxx`

To use them simply add them at the end of the `target_link_libraries`.
Note that they need to be included in reverse order, as they depend on each other.
(Meaning if all are needed, need to add `dlibcxx dlibc dandelion_runtime`)

You can also use the toolchainfile instead by setting up your project with:
```
cmake -DCMAKE_TOOLCHAIN_FILE=<path to toolchainfile> <other args>
```
If you want to use this make sure you have executed the compiler creation script in the SDK folder to create the compilers.

## Debugging
The debug backend will look for a folder called `input_sets` in the directory the executable is run in as well as a folder called `output_sets`.
In those folders if will look for a folder for each set and take each file in a folder in the `input_sets` folder as input to the function.
Folders in the `output_sets` folder will be registered as output sets.
The number of files per folder is limited by length of their names.
We have reserved 4096 bytes for the dirent structures, which include the names, meaning longer names limit how many structures we can read.
In order to avoid recursing folders we express nested files with '+' between folders and files.
For example the input file "test_folder+test_file" will be presented to the file system as "/<set folder name>/test_folder/test_file".

## Building
The target platform and architecture are defined defined as an argument to cmake, i.e.
`cmake .. -DDANDELION_PLATFORM=<platform> -DARCHITECTURE=<architecture>`. Valid values for `<platform>` can be found in [above](#platforms-and-architectures), valid architectures are `x86_64` and `aarch64`.

The default values are `DEBUG` using the architecture of the system that is running the build.
Addtionally we also support `Debug` and `Release` builds with the standard CMake flag `-DCMAKE_BUILD_TYPE=<build type>`

To also build libc and libc++ set the variable `-DNEWLIB=ON`.
For newlib to be built correctly the autoconf version 2.69.
This also enables the build of the in memory file system,
which can also be built without the other newlib builds by setting `-DDANDELION_FS=ON`

## Freestanding
The GCC/Clang standard expects 4 functions to allways be provided in any environment (even freestanding), which allow the compiler to always just insert them.
(https://gcc.gnu.org/onlinedocs/gcc/Standards.html)
Because of this we also provide them.
They are:
- memcpy
- memset
- memmove
- memcmp

Currently they are just implemented somewhat sloppily, could optimize, have better QA or pull in a dependency if this becomes an issue.
