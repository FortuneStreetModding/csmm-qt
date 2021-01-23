# csmm-qt
Custom Street Map Manager cross-platform port

**IMPORTANT: Make sure to use --recursive when cloning this repository, in order to initialized the submodules!**

## Building on Windows (MinGW)

You'll need Cygwin for the mxml dependency. In the `lib/mxml` directory, run `./configure` followed by `make CC=<mingw compiler name>` under Cygwin.

For the yaml-cpp dependency, you don't need Cygwin to build. Make a new directory inside `lib/yaml-cpp` called `build`, `cd` to it, and run `cmake ..` followed by `make`.

Lastly, make sure that openssl is installed on your computer. When deploying, you'll need to distribute the libssl and libcrypto dlls that come with openssl.

The csmm-qt project can then be built the usual way.

## Building on MacOS/Linux

For these OSes, this project depends on libarchive, mxml, and yaml-cpp (which you can install with a package manager of your choice). The project can then be built as usual.
