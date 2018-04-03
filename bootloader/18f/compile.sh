#!/bin/bash

if [ x$1 = x ] ; then
	#echo usage: $0 boardname
	#exit
	for b in ../../boards/* ; do 
		if test -f $b/board.lkr ; then
			echo board=$b
			export BOARD=`basename $b`
			echo $BOARD
			if ! make ; then exit; fi
		fi
	done
	exit
fi

#export BOARD=$1
export BOARD=`basename $1`

if [ `dirname $1` != . ] ; then
	export BOARDDIR=$1
else
	export BOARDDIR=
fi

echo $BOARD $BOARDDIR
make

