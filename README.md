# csmm-qt
Custom Street Map Manager cross-platform port

**IMPORTANT: Make sure to use --recursive when cloning this repository, in order to initialize the submodules!**

## Requirements

- Qt 5.15.2
- MSVC 2019 or later (Windows)
- cmake

## Building on Windows (MinGW)

Make sure that openssl is installed on your computer. When deploying, you'll need to distribute the libssl and libcrypto dlls that come with openssl.

Windows uses the yaml-cpp and libbecquerel submodules in the repo, so there's no need to install these separately.

The project can then be built with CMake.

## Building on MacOS/Linux

For these OSes, this project depends on libarchive, yaml-cpp, and libbecquerel.

libarchive and yaml-cpp can be installed via your favorite package manager.

These OSes use the libbecquerel submodule in the repo, so there's no need to install that separately.

The project can then be built with CMake.
