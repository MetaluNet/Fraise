#!/bin/bash

# $1 = directory to make 
# $2 = pdsend port
# $3 = OS (linux/windows/macos)
echo source: $1
echo port: $2
echo os: $3

PROJDIR=`cd $1 ; pwd -P`
export FRAISEOS=$3
FRAISEH=`dirname $0`/..
#export FRAISE=`readlink -f $FRAISEH`
export FRAISE=`cd $FRAISEH ; pwd -P`
export FRAISEBIN=`cd ${FRAISE}/../FraiseBin;pwd -P`
export PROJ=`basename $PROJDIR`
BOARDLINE=`grep BOARD $PROJDIR/$PROJ.c`
export BOARD=${BOARDLINE##*BOARD}
export BOARD=${BOARD##* }
MODULES_E=
UMODULES_E=

if [ $FRAISEOS = windows ] ; then export PATH=$PATH:${FRAISEBIN}/win32 ; fi


if grep "^#define[[:blank:]][[:blank:]]*NOCLEAN[[:blank:]]*$" $PROJDIR/config.h ; then
	export FRAISECLEAN=
else export FRAISECLEAN=clean
fi

for module in `ls $FRAISE/modules` ; do
	if test -d $FRAISE/modules/$module ; then
		x=`grep $module.h $PROJDIR/$PROJ.c`
		y="${x:1:8}"
		if [ x$y = xinclude ] ; then MODULES_E+="$module "; fi
	fi
done 

for module in `ls $PROJDIR/modules` ; do
	if test -d $PROJDIR/modules/$module ; then
		x=`grep $module.h $PROJDIR/$PROJ.c`
		y="${x:1:8}"
		if [ x$y = xinclude ] ; then UMODULES_E+="$module "; fi
	fi
done 

export MODULES_E
export UMODULES_E

(
	echo "FRAISEOS: $FRAISEOS"
	echo "FRAISE: $FRAISE"
	echo "PROJ: $PROJ"
	echo "PROJDIR: $PROJDIR"
	echo "BOARD: $BOARD"
	echo "MODULES: $MODULES_E"
	echo "USER MODULES: $UMODULES_E"
	if [ -z $FRAISECLEAN ] ; then echo "WILL NOT CLEAN PROJECT." ; fi
	#echo make -C $1 -f $FRAISE/Makefile 
	make -s -C $PROJDIR -f $FRAISE/Makefile 2>&1
) | sed 's/$/;/'|  pdsend $2

#| pdsend $2
