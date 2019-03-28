#!/bin/sh
if [ -n "$TRAVIS_TAG" ] && [ "$TRAVIS_PULL_REQUEST" = "false" ]; then
git clone --branch=master --single-branch --depth 1 https://github.com/devkitPro/3ds-examples examples
cd libctru
doxygen Doxyfile
fi
