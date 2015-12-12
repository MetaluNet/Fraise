#
#  Config for FraiseStep0 ( carte simple face )
#
# ------------- Chip/frequency : -----------------
CHIP=18f2455
FAMILLY=18f
FOSC=48000000 # 48 Mhz


# ------------- Program memory vectors : -----------------
APP_START='0x000'	# application entry point
MEM_SIZE=0x6000

# ------------- Serial : -----------------
# which UART is used ?
UART_PORT=1

# configure the serial driver PIN and TRIS
SERDRV_PIN=LATBbits.LATB0
SERDRV_TRI=TRISBbits.TRISB0

#serial driver pin polarity : state of the pin that enables serial driver
SERDRV_POL=0

#does config.c have setup ?
CONFIG_SETUP=0


