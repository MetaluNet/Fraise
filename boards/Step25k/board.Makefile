#
#  Config for FraiseStep with a pic 18F25K22
#

# ------------- Chip/frequency : -----------------
CHIP=18f25k22
FAMILLY=18f
FOSC=64000000 # 64 Mhz

# ------------- Program memory vectors : -----------------
APP_START='0x0000'	# application entry point
MEM_SIZE=0x8000

# ------------- Serial : -----------------
# which UART is used ?
UART_PORT=1

# configure the serial driver PIN and TRIS
SERDRV_PIN=LATCbits.LATC5
SERDRV_TRI=TRISCbits.TRISC5

#serial driver pin polarity : state of the pin that enables serial driver
SERDRV_POL=0

#does config.c have setup ?
CONFIG_SETUP=1

