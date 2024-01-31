#!/bin/sh
cmake -B build . -Dfraise_board=pico
cd build
make -j8

