#!/bin/bash

CMAKE=../../../../Fraise-toolchain/cmake/bin/cmake

if [ x$1 == x ] ; then
    echo ERROR: no board specified!
    exit
fi

$CMAKE -B build -S . -Dfraise_board=$1
$CMAKE --build build  --parallel 4

