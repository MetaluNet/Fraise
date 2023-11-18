#!/bin/bash

#program project hex file to mchpusb using pk2cmd (linux)

PROJ=FraiseMaster2.1.2
FSUSB=fsusb
HEXFILE=${PROJ}.X/dist/default/production/${PROJ}.X.production.hex

#export OLDPIED=0
echo > config.h
make -C ${PROJ}.X
cp $HEXFILE PiedUsb.hex

#export OLDPIED=1
echo "#define OldPied 1
"> config.h
make -C ${PROJ}.X
cp $HEXFILE oldPiedUsb.hex


#if [ ! -f $HEXFILE ] ; then echo "ERROR : " $HEXFILE "doesn't exist... exiting."; exit ;fi
 
#${FSUSB} --program $HEXFILE
