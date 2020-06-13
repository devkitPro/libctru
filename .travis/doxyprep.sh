#!/bin/bash

if [ ! -e "$DOXY_BINPATH/doxygen" ]; then
	mkdir -p ~/doxygen && pushd ~/doxygen
	wget http://doxygen.nl/files/doxygen-1.8.18.linux.bin.tar.gz
	tar xzf doxygen-1.8.18.linux.bin.tar.gz
	popd
fi
