#!/bin/bash

# $1 = directory where the fruit firmware source is
# $2 = pdsend port

(
	FRAISEH=`dirname $0`/../
	FRAISE=`readlink -f $FRAISEH`
	PROJ=`basename $1`
	BOARDLINE=`grep BOARD $1/main.c`
	BOARD=${BOARDLINE##*BOARD}
	BOARD=${BOARD##* }

	PK2CMD=pk2cmd
	HEXFILE=`dirname $0`/18f/hex/$BOARD.hex
	echo Proj: $PROJ
	echo Board: $BOARD
	echo Hexfile: $HEXFILE
	$PK2CMD -P -M -F$HEXFILE

) | sed 's/$/;/' | pdsend $2
