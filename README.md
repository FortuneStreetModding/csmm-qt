# csmm-qt - Custom Street Map Manager
## Summary
`csmm-qt` is the cross-platform port of Custom Street Map Manager (CSMM). This tool allows modification of Boom Street, Fortune Street, and Itadaki Street Wii disc images. 
## Table of Contents
* [Summary](#summary)
* [Table of Contents](#table-of-contents)
* [Getting Started](#getting-started)
    * [Installing](#installing)
    * [Using CSMM](#using-csmm)
* [Building from Source](#building-from-source)
    * [Build Requirements](#build-requirements)
    * [Initial Steps](#initial-steps)
    * [Windows](#windows)
    * [MacOS](#macos)
    * [Linux](#linux)
* [Contributing](#contributing)
## Getting Started
### Installing
Go to the [Releases](https://github.com/FortuneStreetModding/csmm-qt/releases) page and download the latest version for your system. Linux users will also need to install [libssl](https://packages.ubuntu.com/focal-updates/libssl1.1) for network functionality. 

### Using CSMM
For help using CSMM, please see the [CSMM User Manual](https://github.com/FortuneStreetModding/fortunestreetmodding.github.io/wiki/CSMM-User-Manual).

## Building from Source

**IMPORTANT: Make sure to use --recursive when cloning this repository, in order to initialize the submodules!**

### Build Requirements

- Qt 5.15.2
- CMake
- MSVC 2019 or later (Windows)

### Initial Steps
To start, clone this repository recursively Adding the `--recursive` flag is important as doing so will initialize the linked submodules:

`git clone git@github.com:FortuneStreetModding/csmm-qt.git --recursive`

### Windows
Before you start, ensure [OpenSSL](https://www.openssl.org/) is installed on your computer. When you deploy the Windows executable, you'll need to copy the `libssl` and `libcrypto` DLLs that come with OpenSSL into the binary's directory once the application has been built.

Windows uses the `yaml-cpp` and `libbecquerel` submodules in this repo, so there's no need to install these dependencies separately.

Just open `CMakeLists.txt` with Qt Creator and build the project with CMake.

### MacOS
Mac users can most easily install `csmm-qt`'s dependencies using Homebrew:

`brew install yaml-cpp` 

`brew install libarchive`

`libbecquerel` does not need to be installed, as the submodule linked in this repo is used instead.

Once the dependencies are installed, you can proceed to open `CMakeLists.txt` with Qt Creator and build the project with CMake.

### Linux
Ubuntu, Linux Mint, and Debian users can most easily install `csmm-qt`'s dependencies with `apt-get`:

`sudo apt-get install -y libyaml-cpp-dev libarchive-dev wget ninja-build libxkbcommon0 libxkbcommon-x11-0`

Users of other Linux distributions should install the above dependencies with thier favorite package manager. Just as it is with Mac OS, the Linux build process also uses the `libbecquerel` submodule linked in this repo, so it does not need to be installed separately.

Once the dependencies are installed, you can proceed to open `CMakeLists.txt` with Qt Creator and build the project with CMake.

## Contributing
We welcome contributions! If you would like to contribute to the development of `csmm-qt`, please feel free to [submit a PR](https://github.com/FortuneStreetModding/csmm-qt/pulls) with your changes, create an [Issue](https://github.com/FortuneStreetModding/csmm-qt/issues) to request a change, or join our [Discord server](https://discord.gg/DE9Hn7T) to further discuss the future of Fortune Street modding!