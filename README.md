# csmm-qt
Custom Street Map Manager cross-platform port

**IMPORTANT: Make sure to use --recursive when cloning this repository, in order to initialize the submodules!**

## Building on Windows (MinGW)

You'll need Cygwin for the mxml dependency. In the `lib/mxml` directory, run `./configure` followed by `make CC=<mingw compiler name>` under Cygwin.

For the yaml-cpp dependency, you don't need Cygwin to build. Make a new directory inside `lib/yaml-cpp` called `build`, `cd` to it, and run `cmake ..` followed by `make`.

Lastly, make sure that openssl is installed on your computer. When deploying, you'll need to distribute the libssl and libcrypto dlls that come with openssl.

The csmm-qt project can then be built the usual way.

## Building on MacOS/Linux

For these OSes, this project depends on libarchive, mxml, the Boehm garbage collector (bdwgc), and yaml-cpp.

libarchive and yaml-cpp can be installed via your favorite package manager.

On MacOS, Homebrew's mxml causes major crashing problems, so you'll have to run `./configure` followed by `make` in the `lib/mxml` directory. On non-MacOS Unix-like systems you can probably get away with using a package manager here.

On MacOS, you can install the Boehm garbage collector with `brew install bdw-gc` if you have Homebrew. Otherwise you'll have to cd to `lib/bdw-gc`, and run `./autogen.sh`, `./configure`, and `make` in that order.

The project can then be built as usual.
