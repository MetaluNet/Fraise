#!/bin/bash

DONE=
NOLKR=
COMPILEERROR=

if [ x$1 = x ] ; then
	#echo usage: $0 boardname
	#exit
	for b in ../../boards/*/ ; do 
		export BOARD=`basename $b`
		echo board=$BOARD
		if test -f $b/board.lkr ; then
			if make &> /dev/null ; then DONE+=$BOARD" " ; else COMPILEERROR+=$BOARD" " ; fi
		else NOLKR+=$BOARD" " ; fi
	done
	
	echo "boards missing board.lkr: " $NOLKR
	echo "boards with compilation error: " $COMPILEERROR
	echo "boards successfully compiled: " $DONE 
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

