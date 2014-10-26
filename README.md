ctrulib
=======

CTR User Library

library for writing user mode arm11 code for the 3DS (CTR)

the goal with this is to create a very straightforward interface with the 3DS's OS.
it is not meant to provide higher level functions; to put things in perspective, the purpose of ctrulib would be to sit between the OS and a possible port of SDL rather than replace it.

setup
=======

ctrulib is just a library and needs a toolchain to function. we built ctrulib to be used in conjunction with devkitARM. you may find instructions on how to install devkitARM here : http://devkitpro.org/wiki/Getting_Started

once devkitARM is installed, you will need to manually patch it with the following : http://mtheall.com/~fincs/3dsdkA/

that done, just checkout ctrulib, build it, set your CTRULIB path and you should be good to go !

license
=======

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
