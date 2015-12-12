//
//  Config for 8X2A
//

// ------------- Chip/frequency : -----------------
#define CHIP 18f26k22
#define FAMILLY 18f
#define FOSC 64000000 // 64 Mhz

// ------------- Program memory vectors : -----------------
//APP_START='0x0000'	# application entry point
#define MEM_SIZE 0x10000

// ------------- Serial : -----------------
// which UART is used ?
#define UART_PORT 2

// configure the serial driver PIN and TRIS
#define SERDRV_PIN LATCbits.LATC4
#define SERDRV_TRI TRISCbits.TRISC4

//serial driver pin polarity : state of the pin that enables serial driver
#define SERDRV_POL 0

//does config.c have setup ?
#define CONFIG_SETUP 1

