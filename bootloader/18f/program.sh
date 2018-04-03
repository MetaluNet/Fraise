#!/bin/bash

if [ x$1 = x ] ; then
	echo usage: $0 boardname
	exit
	fi

export BOARD=`basename $1`

if [ `dirname $1` != . ] ; then
	export BOARDDIR=$1
else
	export BOARDDIR=
fi

make progpk2

