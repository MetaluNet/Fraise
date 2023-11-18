#
#  Makefile for Fraise Pied25k50
#

CHIP=18f25k50


SDCCHOME = ../../bin
SDCC = $(SDCCHOME)/linux64/sdcc
SDCCFLAGS = -mpic16 -p$(CHIP) --disable-warning 85 --std-sdcc99 --obanksel=3 --use-non-free \
			-I$(SDCCHOME)/share/include/pic16 -I$(SDCCHOME)/share/non-free/include/pic16 \
			-L$(SDCCHOME)/share/non-free/lib/pic16 -L$(SDCCHOME)/share/lib/pic16

CDCACM=usbcdcacm1.1/

SRCS= $(CDCACM)/usbcdc.c $(CDCACM)/printft.c fraisemaster/fraisemaster.c main.c
      

HDRS= fraisemaster/fraisemaster.h config.h prj_usb_config.h \
	$(CDCACM)/usbcdc_defs.h $(CDCACM)/usb_defs.h $(CDCACM)/usbcdc.h $(CDCACM)/usbpic_defs.h

OBJS =	$(SRCS:%.c=%.o)
ASMS =	$(SRCS:%.c=%.asm)
LSTS =	$(SRCS:%.c=%.lst)

CRT = crt0iz-rloc
#PROJ=PiedUsb
PROJ=Pied25k50

CC=$(SDCC)
DIS = gpdasm
FSUSB = fsusb

#OPTS= --model-small --vc --denable-peeps --optimize-cmp --optimize-df --obanksel=9 --opt-code-size --fommit-frame-pointer --ivt-loc=0x0800 --use-crt=$(CRT).o

OPTS= --vc --denable-peeps --optimize-cmp --optimize-df --obanksel=9 --opt-code-size --fomit-frame-pointer 

#--use-crt=$(CRT).o

#--optimize-goto 
#OPTS= --no-peep --fstack

CFLAGS= -I. -I$(CDCACM) -Ifraisemaster $(OPTS) $(SDCCFLAGS)  
#--std-c99 
LINKER_SCRIPT= sd$(CHIP)-full.lkr

LIBS= libsdcc.lib libc18f.lib libio$(CHIP).lib libdev$(CHIP).lib

# .SUFFIXES: .c .asm .o .hex

all: $(PROJ).hex

$(OBJS):	%.o: %.c  Makefile $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(PROJ).hex: $(OBJS) $(LINKER_SCRIPT) $(CRT).o
	$(CC) $(CFLAGS) -L $(SDCC_HOME)/lib/pic16 -Wl,"-m -C -w -s $(LINKER_SCRIPT)" -o $(PROJ) $(OBJS) $(LIBS)

disass:
	$(DIS) -p p$(CHIP) $(PROJ).hex > $(PROJ).lis

progpk2: all
	pk2cmd -f$(PROJ).hex -p /m /e /j /r

clean:
	rm -f $(OBJS) $(ASMS) $(LSTS) core .depend depend $(PROJ).cod $(PROJ).lst *.o *.asm *.lst *.map

$(CRT).o:	%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

dummy:

