#!/bin/bash

if [ x$1 = x ] ; then
	echo usage: $0 boardname
	exit
	fi

export BOARD=$1

make progpk2

