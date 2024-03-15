#!/bin/bash

# $1 = directory where the fruit firmware source is
# $2 = pdsend port

(
	FRAISEDIR=$(readlink -f `dirname $0`/../../..)
	PROJ=`basename $1`
	BOARDLINE=`grep BOARD $1/main.c`
	BOARD=${BOARDLINE##*BOARD}
	BOARD=${BOARD##* }

	PK2CMD=pk2cmd
	HEXFILE=$FRAISEDIR/boards/$BOARD/$BOARD.hex
	echo Proj: $PROJ
	echo Board: $BOARD
	echo Hexfile: $HEXFILE
	$PK2CMD -P -M -B$FRAISEDIR/pic/sys/bootloader/pk2cmd -F$HEXFILE

) | sed 's/$/;/' | pdsend $2
