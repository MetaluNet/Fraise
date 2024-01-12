#!/bin/bash

if [ x$1 == x ] ; then
	echo ERROR: no board specified!
	exit
fi

cmake -B build -S . -Dfraise_board=$1
cd build
make -j8

