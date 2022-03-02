# csmm-qt
Custom Street Map Manager cross-platform port

**IMPORTANT: Make sure to use --recursive when cloning this repository, in order to initialize the submodules!**

## Requirements

- Qt 5.15.2
- mingw810_64 (Windows)

## Building on Windows (MinGW)

For the yaml-cpp dependency, you don't need Cygwin to build. Make a new directory inside `lib/yaml-cpp` called `build`, `cd` to it, and run `cmake .. -G "MinGW Makefiles"` followed by `mingw32-make`.

To build libbecquerel, make a new directory inside `lib/libbecquerel` called `build`, `cd` to it, and run `cmake .. -G "MinGW Makefiles"` followed by `mingw32-make`.

Lastly, make sure that openssl is installed on your computer. When deploying, you'll need to distribute the libssl and libcrypto dlls that come with openssl.

The csmm-qt project can then be built the usual way.

## Building on MacOS/Linux

For these OSes, this project depends on libarchive, yaml-cpp, and libbecquerel.

libarchive and yaml-cpp can be installed via your favorite package manager.

To build libbecquerel, make a new directory inside `lib/libbecquerel` called `build`, `cd` to it, and run `cmake ..` followed by `make`.
