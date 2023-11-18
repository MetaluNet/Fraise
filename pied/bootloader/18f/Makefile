#
#  Makefile for FraiseBootloader
#	to build MyBoard.hex : 
# export BOARD=MyBoard ; make

MAINFRAISE=../../..

SDCC_SHARE=$(MAINFRAISE)/pied/bin/share
SDCC=$(MAINFRAISE)/pied/bin/linux/sdcc
CC=$(SDCC)
PK2CMD= pk2cmd
SHELL=/bin/bash

BOARDSDIR=$(MAINFRAISE)/boards

ifeq ($(BOARDDIR),)
	BOARDDIR=$(BOARDSDIR)/$(BOARD)
endif

BOARDMAKEFILE:=$(BOARDDIR)/board.Makefile
include $(BOARDMAKEFILE)

HEXFILE= $(BOARDDIR)/$(BOARD).hex

BLD_START=$(shell echo "$$(($(MEM_SIZE) - (32 * 64)))")

ifeq ($(BOOT_TIME),)
	BOOT_TIME = 1000
endif

ifndef APP_START
	APP_START = 0
endif

SRCS= ./src/fbld.c $(BOARDDIR)/boardconfig.c

OBJS =	$(SRCS:%.c=%.o)
ASMS =	$(SRCS:%.c=%.asm)
LSTS =	$(SRCS:%.c=%.lst)
CODS =  $(SRCS:%.c=%.cod)

OPTS = --ivt-loc=$(BLD_START) --no-crt
CFLAGS= -I. -I./lib -I$(SDCC_SHARE)/include/pic16 \
		-I$(SDCC_SHARE)/non-free/include/pic16 $(OPTS) -mpic16 -p$(CHIP) \
		--vc --denable-peeps  --optimize-cmp --optimize-df --obanksel=2 \
		--opt-code-speed --fomit-frame-pointer --use-non-free
MACROS= -DFOSC=$(FOSC) -DBLD_START=$(BLD_START) -DAPP_START=$(APP_START) \
		-DUART_PORT=$(UART_PORT) -DSERDRV_PIN=$(SERDRV_PIN) \
		-DSERDRV_TRI=$(SERDRV_TRI) -DSERDRV_POL=$(SERDRV_POL) \
		-DCONFIG_SETUP=$(CONFIG_SETUP) -DBOOTLOADER -DBOOT_TIME=$(BOOT_TIME)

LIBS= libsdcc.lib libc18f.lib

SUFFIXES= obj lst asm o cod

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
	$(CC) $(CFLAGS) $(MACROS) -L $(SDCC_SHARE)/lib/pic16 -L $(SDCC_SHARE)/non-free/lib/pic16 \
		-o $(HEXFILE) $(OBJS) $(LIBS) -Wl"-s $(CHIP).lkr"

disass:
	$(DIS) -p p$(CHIP) $(HEXFILE) > $(PROJ).lis

progpk2:
	$(PK2CMD) -P -M -F$(HEXFILE)

clean:
	rm -f $(foreach suf,$(SUFFIXES),$(BOARDSDIR)/*/*.$(suf), $(BOARDDIR)/*.$(suf))
	rm -f src/*.o src/*.asm src/*.lst src/*.cod hex/*.cod hex/*.lst hex/*.asm

