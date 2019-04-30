#!/bin/sh
git clone --branch=master --single-branch --depth 1 https://github.com/devkitPro/3ds-examples examples
cd libctru
doxygen Doxyfile
