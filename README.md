# csmm-qt
Custom Street Map Manager cross-platform port

**IMPORTANT: Make sure to use --recursive when cloning this repository, in order to initialize the submodules!**

## Building on Windows (MinGW)

For the mxml dependency you need Git for Window's `bash` (generally under `C:\Program Files\Git\bin` or the like) or some other `bash` shell. In the `lib/mxml` directory, run `./configure` followed by `mingw32-make`.

For the yaml-cpp dependency, you don't need Cygwin to build. Make a new directory inside `lib/yaml-cpp` called `build`, `cd` to it, and run `cmake .. -G "MinGW Makefiles"` followed by `mingw32-make`.

For bdwgc, you should cd to `lib/bdwgc`, and run `cmake . -G "MinGW Makefiles" -Denable_threads=OFF` (this is different from yaml-cpp's command) followed by by `mingw32-make`.

Lastly, make sure that openssl is installed on your computer. When deploying, you'll need to distribute the libssl and libcrypto dlls that come with openssl.

The csmm-qt project can then be built the usual way.

## Building on MacOS/Linux

For these OSes, this project depends on libarchive, mxml, the Boehm garbage collector (bdwgc), and yaml-cpp.

libarchive and yaml-cpp can be installed via your favorite package manager.

Mxml seems to cause major crashing problems when it is a shared library, so you'll have to run `./configure` followed by `make` in the `lib/mxml` directory.

On MacOS, you can install the Boehm garbage collector with `brew install bdw-gc` if you have Homebrew. Otherwise you'll have to cd to `lib/bdw-gc`, and run `./autogen.sh`, `./configure`, and `make` in that order.

The project can then be built as usual.
