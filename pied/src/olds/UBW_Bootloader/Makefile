#
#  Makefile for Microchip sdcc project
#

CHIP=18f2455

# make /usr if you want released tools:
#MY_BASE=/home/antoine/Datas/Sources/MicroPIC/sdcc/
MY_BASE=/usr/

SDCC_HOME=$(MY_BASE)/share/sdcc
SDCC=$(MY_BASE)/bin/sdcc
CC=$(SDCC)

SRCS= main.c
#config.c
HDRS= main.h

OBJS =	$(SRCS:%.c=%.o)
ASMS =	$(SRCS:%.c=%.asm)
LSTS =	$(SRCS:%.c=%.lst)

#CRT = crt0iz-rloc
CRT = crt0iz
#PROJ=FW_B-sansconfig
PROJ=FW_B-BootProtect

DIS = gpdasm
#FSUSB = fsusb
#/home/antoine/PicProjects/USB/fsusb-0.1.11-2/fsusb
#PICP= ~/Datas/Sources/MicroPIC/picp-0.6.8.n/picp
#PICP=/home/arsene/Datas/FromAntoine/PicProg/picp-0.6.8.n/picp
#PICP=/home/arsene/Datas/FromAntoine/PicProg/picp-0.6.8.n/picp
PICP=/home/antoine/Documents/Pic/picp-0.6.8.n/picp
#PK2CMD= ../../../bin/linux/pk2cmd/pk2cmd
PK2CMD= pk2cmd

OPTS= --use-crt=$(CRT).o 
#OPTS= --no-peep --fstack 
#--ivt-loc=0x0800
#--model-small --vc --denable-peeps --optimize-goto --optimize-cmp --optimize-df --obanksel=9 --opt-code-size --fommit-frame-pointer 
CFLAGS= -I. -I$(SDCC_HOME)/include/pic16 $(OPTS) -mpic16 -pp$(CHIP) --std-sdcc99 --model-small --vc --denable-peeps --optimize-goto --optimize-cmp --optimize-df --obanksel=2 --opt-code-speed --fommit-frame-pointer

LINKER_SCRIPT= $(CHIP).lkr
LIBS= libsdcc.lib libc18f.lib libio$(CHIP).lib pic$(CHIP).lib

# .SUFFIXES: .c .asm .o .hex

all: $(PROJ).hex

$(OBJS):	%.o: %.c  Makefile
	$(CC) $(CFLAGS) -c $< -o $@

#$(PROJ).hex:	$(OBJS) $(LINKER_SCRIPT)
#	$(CC) $(CFLAGS) -L $(SDCC_HOME)/lib/pic16 -Wl,"-m -w -s $(LINKER_SCRIPT)" -o $(PROJ) $(OBJS) $(LIBS)

disass:
	$(DIS) -p p$(CHIP) $(PROJ).hex > $(PROJ).lis

progpicp: $(PROJ).hex
#	su root -c " $(FSUSB) --program $(PROJ).hex "
#	$(FSUSB) --program $(PROJ).hex
	$(PICP) /dev/ttyUSB0 $(CHIP) -ef -wp $(PROJ).hex -wc  0x0220 0x1e32 0x8000 0x0081 0xc00f 0xe00f 0x400f
#							      0x0224=quartz20MHz 0x0220=4MHz


#								0x0e20 0x001a 0x0500 0x0080 0xc00f 0xe00f 0x400f
#									      0x0400=CCP2 on RB3; 0x0500 on RC1
#								0x0e24=quartz20MHz 0x0e20=4MHz

progpk2: $(PROJ).hex
	$(PK2CMD) -P -M -F$(PROJ).hex

clean:
	rm -f $(OBJS) $(ASMS) $(LSTS) core .depend depend $(PROJ).cod $(PROJ).hex $(PROJ).lst

#$(CRT).o:	%.o: %.c
#	$(CC) $(CFLAGS) -c $< -o $@

config.hex: config.c
	$(CC) $(CFLAGS) --no-crt -o config config.c 

#depend .depend dep:
#	for file in $(SRCS); do DIR=`dirname $$file`; echo -n $$DIR/ >> $@; $(CC) $(CFLAGS) -M $$file >> $@; done

#archive:
#	cd ..; find D_143 -depth -not -type d -not -lname "*" -not -path "*~"  > /tmp/backup3.files.txt; tar -c -j --files-from=/tmp/backup3.files.txt --file=$(HOME)/in/D_143.sdcc.$(VERSION).tar.bz2

#dummy:
# include a dependency file if one exists
#ifeq (.depend,$(wildcard .depend))
#include .depend
#endif
