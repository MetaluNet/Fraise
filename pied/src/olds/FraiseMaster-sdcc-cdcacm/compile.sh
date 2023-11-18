#!/bin/bash

#program project hex file to mchpusb using pk2cmd (linux)

#PROJ=FraiseMaster2.1.2
FSUSB=fsusb
HEXFILE=Project.hex

#export OLDPIED=0
echo > config.h
make clean
make
#-C ${PROJ}.X
cp $HEXFILE PiedUsb.hex
exit
#export OLDPIED=1
echo "#define OldPied 1
"> config.h
#make -C ${PROJ}.X
make clean
make
cp $HEXFILE oldPiedUsb.hex

make clean

#if [ ! -f $HEXFILE ] ; then echo "ERROR : " $HEXFILE "doesn't exist... exiting."; exit ;fi
 
#${FSUSB} --program $HEXFILE
