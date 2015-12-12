#ifndef _8X2A_IO_
#define _8X2A_IO_

#define MOTA_EN  			LATAbits.LATA4
#define MOTA_IN1 			LATAbits.LATA3
#define MOTA_IN2 			LATAbits.LATA2
#define SETPORT_MOTA do{ TRISAbits.TRISA4=TRISAbits.TRISA3=TRISAbits.TRISA2=0;} while(0)

#define MOTB_EN 			LATBbits.LATB5
#define MOTB_IN1 			LATAbits.LATA1
#define MOTB_IN2 			LATAbits.LATA0
#define SETPORT_MOTB do{ TRISBbits.TRISB5=TRISAbits.TRISA1=TRISAbits.TRISA0=0;} while(0)


#define MOTC_EN 			LATCbits.LATC2
#define MOTC_IN1 			LATBbits.LATB4
#define MOTC_IN2 			LATAbits.LATA6
#define SETPORT_MOTC do{ TRISCbits.TRISC2=TRISBbits.TRISB4=TRISAbits.TRISA6=0;} while(0)

#define MOTD_EN 			LATCbits.LATC0
#define MOTD_IN1 			LATBbits.LATB3
#define MOTD_IN2 			LATAbits.LATA7
#define SETPORT_MOTD do{ TRISCbits.TRISC0=TRISBbits.TRISB3=TRISAbits.TRISA7=0;} while(0)

#define K1PORT 	C
#define K1BIT 	1

#define K2PORT	C
#define K2BIT 	3
#define K2AN	15

#define K3PORT	C
#define K3BIT 	4
#define K3AN	16

#define K4PORT	C
#define K4BIT 	5
#define K4AN	17

#define K5PORT	C
#define K5BIT 	6
#define K5AN	18

#define K6PORT	C
#define K6BIT 	7
#define K6AN	19

#define K7PORT	B
#define K7BIT 	0
#define K7AN	12

#define K8PORT	B
#define K8BIT 	1
#define K8AN	10

#define K9PORT	B
#define K9BIT 	2
#define K9AN	8


#define MA1PORT	A
#define MA1BIT	3
#define MA1AN	3

#define MA2PORT	A
#define MA2BIT	2
#define MA2AN	2

#define MB1PORT	A
#define MB1BIT	1
#define MB1AN	1

#define MB2PORT	A
#define MB2BIT	0
#define MB2AN	0

#define MC1PORT	B
#define MC1BIT	4
#define MC1AN	11

#define MC2PORT	A
#define MC2BIT	6

#define MD1PORT	B
#define MD1BIT	3
#define MD1AN	9

#define MD2PORT	A
#define MD2BIT	7

#define SERIAL_UART_PORT 1
#define SERIAL_TX_PIN K5
#define SERIAL_RX_PIN K6


#endif
