#!/bin/bash
export DEVKITARM=/Users/aurelio/Programmazione/NintendoDS/devkitARM
export DEVKITPRO=/Users/aurelio/Programmazione/NintendoDS
export CTRULIB=/Users/aurelio/Programmazione/NintendoDS/ctrulib/libctru
make clean
make
sh copy.sh