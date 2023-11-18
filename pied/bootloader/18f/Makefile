#
#  Makefile for FraiseBootloader
#	to build MyBoard.hex : 
# export BOARD=MyBoard ; make

MAINFRAISE=../..

	SDCC_SHARE=$(MAINFRAISE)/bin/share
	SDCC=$(MAINFRAISE)/bin/linux/sdcc
	CC=$(SDCC)
	PK2CMD= pk2cmd
	SHELL=/bin/bash

#	SDCC_SHARE=$(MY_BASE)/share/sdcc
#	SDCC=$(MY_BASE)/bin/sdcc
#	CC=$(SDCC)
#	DIS = gpdasm
#	FSUSB = fsusb
#	PICP=picp
#	PK2CMD= pk2cmd
#	SHELL=/bin/bash

BOARDSDIR=$(MAINFRAISE)/boards


ifeq ($(BOARDDIR),)
	BOARDDIR=$(BOARDSDIR)/$(BOARD)
endif

BOARDMAKEFILE:=$(BOARDDIR)/board.Makefile
BLD_START=`echo print "$(MEM_SIZE)-(32*64)" | python` 

include $(BOARDMAKEFILE) 

ifndef APP_START
	APP_START = 0
endif

SRCS= ./src/fbld.c $(BOARDDIR)/boardconfig.c

OBJS =	$(SRCS:%.c=%.o)
ASMS =	$(SRCS:%.c=%.asm)
LSTS =	$(SRCS:%.c=%.lst)
CODS =  $(SRCS:%.c=%.cod)

OPTS = --ivt-loc=$(BLD_START) --no-crt
#--model-small --vc --denable-peeps --optimize-goto --optimize-cmp --optimize-df --obanksel=9 --opt-code-size --fommit-frame-pointer --std-sdcc99
CFLAGS= -I. -I./lib -I$(SDCC_SHARE)/include/pic16 -I$(SDCC_SHARE)/non-free/include/pic16 $(OPTS) -mpic16 -p$(CHIP)  \
		--vc --denable-peeps  --optimize-cmp --optimize-df --obanksel=2 \
		--opt-code-speed --fomit-frame-pointer --use-non-free
MACROS= -DFOSC=$(FOSC) -DBLD_START=$(BLD_START) -DAPP_START=$(APP_START) \
		-DUART_PORT=$(UART_PORT) -DSERDRV_PIN=$(SERDRV_PIN) \
		-DSERDRV_TRI=$(SERDRV_TRI) -DSERDRV_POL=$(SERDRV_POL) \
		-DCONFIG_SETUP=$(CONFIG_SETUP) -DBOOTLOADER -DBOOT_TIME=$(BOOT_TIME)
#-I$(SDCC_SHARE)/non-free/include/pic16		

#LIBS= libsdcc.lib libc18f.lib libio$(CHIP).lib pic$(CHIP).lib
LIBS= libsdcc.lib libc18f.lib
#libio$(CHIP).lib 
# .SUFFIXES: .c .asm .o .hex
HEXFILE= hex/$(BOARD).hex

SUFFIXES= obj lst asm o
	
all: $(HEXFILE) clean
	 make clean
	
$(BOARD): all

$(BOARDMAKEFILE): 
	@ echo no board declared

$(OBJS): %.o: %.c  Makefile $(BOARDMAKEFILE) src/eeprom.c
	@echo ---- Compile $@ ----
	$(CC) $(CFLAGS) $(MACROS) -c $< -o $@

$(HEXFILE): ./src/fbld.c ./src/fbld.o $(BOARDDIR)/boardconfig.c $(OBJS) $(BOARDMAKEFILE) Makefile
	@echo ---- Linking and building $@ ----
	$(CC) $(CFLAGS) $(MACROS) -L $(SDCC_SHARE)/lib/pic16  -L $(SDCC_SHARE)/non-free/lib/pic16 -o $(HEXFILE) $(OBJS) $(LIBS) -Wl"-s $(CHIP).lkr"

# -O 1 -m -w 
	
disass:
	$(DIS) -p p$(CHIP) $(HEXFILE) > $(PROJ).lis

progpicp: all
#	su root -c " $(FSUSB) --program $(PROJ).hex "
	$(FSUSB) --program $(HEXFILE)
#	$(PICP) /dev/ttyUSB0 $(CHIP) -ef -wp $(PROJ).hex -wc 0x0e20 0x001a 0x0500 0x0080 0xc00f 0xe00f 0x400f
#									   0x0400=CCP2 on RB3; 0x0500 on RC1
#							     0x0e24=quartz20MHz 0x0e20=4MHz

progpk2:
	$(PK2CMD) -P -M -F$(HEXFILE)

clean:
	rm -f $(foreach suf,$(SUFFIXES),$(BOARDSDIR)/*/*.$(suf), $(BOARDDIR)/*.$(suf)) src/*.o src/*.asm src/*.lst src/*.cod hex/*.cod hex/*.lst hex/*.asm

#	rm -f */*.o */*.asm */*.lst src/*.o src/*.asm src/*.lst src/*.cod hex/*.cod hex/*.lst hex/*.asm


