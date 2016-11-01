#!/bin/bash

# $1 = directory to make
# $2 = pdsend port

(
	FRAISEH=`dirname $0`/../
	FRAISE=`readlink -f $FRAISEH`
	PROJ=`basename $1`
	BOARDLINE=`grep BOARD $1/main.c`
	BOARD=${BOARDLINE##*BOARD}
	BOARD=${BOARD##* }

	echo Proj: $PROJ
	echo Board: $BOARD

	PK2CMD=pk2cmd
	HEXFILE=`dirname $0`/../bootloader/18f/hex/$BOARD.hex
	$PK2CMD -P -M -F$HEXFILE

) | sed 's/$/;/' | pdsend $2
