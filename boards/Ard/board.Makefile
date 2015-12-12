#
#  Config for 8X2A
#

# ------------- Chip/frequency : -----------------
CHIP=18f26k22
FAMILLY=18f
FOSC=64000000 # 64 Mhz

# ------------- Program memory vectors : -----------------
#APP_START='0x0000'	# application entry point
MEM_SIZE=0x10000

# ------------- Serial : -----------------
# which UART is used ?
UART_PORT=2

# configure the serial driver PIN and TRIS
SERDRV_PIN=LATAbits.LATA7
SERDRV_TRI=TRISAbits.TRISA7

#serial driver pin polarity : state of the pin that enables serial driver
SERDRV_POL=0

#does config.c have setup ?
CONFIG_SETUP=1

