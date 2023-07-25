A CMake file for building the library for the Gameboy Advance.

1. Make sure to have [gba-toolchain](https://github.com/felixjones/gba-toolchain/) installed
2. `cd` into this directory and invoke Cmake with the following command: `cmake . --toolchain /path/to/arm-gba-toolchain.cmake`
3. run `make` to build the shared library, which should result in a `libcson.a` file.
4. Include the library in a GBA Project by specifying `target_link_libraries(first PRIVATE /path/to/libcson.a)` in that project's `CMakeLists.txt`.
