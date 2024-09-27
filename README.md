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
- `KVM`, for the kvm-based platform
- `WASM`, for the webassembly platform
- `DEBUG` (default), for running applications locally on a linux system with mock inputs,
dumping outputs to the terminal when terminating.

If no value is set, `DEBUG` will be used for the platform value.

To enable debug build set the cmake variable `CMAKE_BUILD_TYPE` to `Debug`.
This can be done by adding `-DCMAKE_BUILD_TYPE=Debug` to the cmake command or setting it as an env variable. 

## Debugging
When the code is built for the debug backend it expects a debug_config.txt in the folder it is run.
In that file it expects the following:
- a line with the input sets formatted as:
    - a positive integer followed by a space, numbering how many input set names there are
    - a list of input set names consisting only of characters and underscores followed a space, the number of input buffers belonging to this set and another space
    - a newline character
- a line with the output sets formatted as:
    - a positive integer followed by a space to indicate how many output sets there are
    - a list output set names consisting of only character and underscores followed by a space
    - a newline character
- a line per input buffers formatted as
    - name consising of only characters and underscores followed by a space
    - positive interger that is used as key followed by a space
    - a string in quotes "<data here>", followed by the newline

## Interface expectations
### libc
When using libc or any system on top of it values can be fed into stdin, argv and environ by specifying a input set called "stdio".
The system will look for items called "stdin", "argv" and "environ" in that set and automaticall make it available on the expected methods.
For argv and environ the content of the item is expected to be space separated as it would be on the command line.
To tell the programm it is called cat and has arguments foo and bar the total content of argv should be "cat foo bar" (without quotation marks).
Similarly for environ, if foo should be set to 2 and bar should be set to 3 the content of environ should be "foo=2 bar=3"
Currently there is no variable subsitution so defining foo to be 2 and then using foo in a later definition does not work.
Both the '' and "" can be used to escape strings that contain spaces.
Escaping the quote characters with backslashes is currently not supported, but they can be escaped by wrapping in the other type of quotes,
as everything within the outermost set of quotataion marks will be esacaped.
Example: `'"test"'` will become `"test"` 
Multiple quoted strings without a space in between are considered a single argument.
Example: `'test'"test"` will become `testtest`

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