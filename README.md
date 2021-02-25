# libctru - CTR User Library

![Build Status](https://github.com/devkitPro/libctru/actions/workflows/build.yaml/badge.svg)

Library for writing user mode ARM11 code for the 3DS (CTR)

This library aims to provide the foundations necessary to write 3DS Homebrew, and straightforwardly access the different functionalities provided by the 3DS operating system.
It is not meant to provide higher level functions; to put things in perspective, the purpose of libctru would be to sit between the OS and a possible port of SDL rather than replace it.

*(Originally located at github.com/smealum/ctrulib)*

# Setup

libctru is just a library and needs a toolchain to function. devkitARM (created by [devkitPro](http://devkitpro.org)) is the officially supported ARM cross compiling toolchain, which provides the framework necessary to supply a usable POSIX-like environment, with working C and C++ standard libraries; as well as the tools required to compile homebrew in the 3DSX format, and assemble GPU shaders. The use of other ARM toolchains is severely discouraged.

The most recent version of devkitARM is always recommended. The [installers/setup scripts](https://devkitpro.org/wiki/devkitPro_pacman) supplied by devkitPro install a prebuilt copy of the latest stable version of libctru, which is recommended for general use. Please note that devkitPro has a policy of keeping legacy code to a minimum, so a library upgrade may result in older code failing to compile or behave properly. Developers are encouraged to keep their code working with the latest versions of the tools and libraries.

You may find instructions on how to install devkitARM on [the devkitPro Wiki](http://devkitpro.org/wiki/Getting_Started).

# Documentation

The documentation is automatically built upon release and can be found at the following url: https://devkitpro.github.io/libctru/

# License

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
     distribution.
