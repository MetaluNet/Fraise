#!/bin/bash

if [ x$1 == x ] ; then
	echo ERROR: no board specified!
	exit
fi

../../../../Fraise-toolchain/cmake/bin/cmake -B build -S . -Dfraise_board=$1
cd build
make -j8

