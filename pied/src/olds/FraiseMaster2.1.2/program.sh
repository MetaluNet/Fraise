#!/bin/bash

#program project hex file to mchpusb using pk2cmd (linux)

PROJ=FraiseMaster2.1.2
FSUSB=fsusb
if [ x$1 == xold ]; then HEXFILE=oldPiedUsb.hex
else HEXFILE=PiedUsb.hex
fi

#HEXFILE=${PROJ}.X/dist/default/production/${PROJ}.X.production.hex

if [ ! -f $HEXFILE ] ; then echo "ERROR : " $HEXFILE "doesn't exist... exiting."; exit ;fi
echo Using hex file : $HEXFILE
${FSUSB} --program $HEXFILE
