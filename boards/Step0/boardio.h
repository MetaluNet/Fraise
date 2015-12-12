#ifndef _8X2A_IO_
#define _8X2A_IO_

#define MOTA_EN  			LATAbits.LATA4
#define MOTA_IN1 			LATBbits.LATB7
#define MOTA_IN2 			LATBbits.LATB6
#define SETPORT_MOTA do{ TRISAbits.TRISA4=TRISBbits.TRISB7=TRISBbits.TRISB6=0;} while(0)

#define MOTB_EN 			LATAbits.LATA5
#define MOTB_IN1 			LATBbits.LATB5
#define MOTB_IN2 			LATBbits.LATB4
#define SETPORT_MOTB do{ TRISAbits.TRISA5=TRISBbits.TRISB5=TRISBbits.TRISB4=0;} while(0)

#define K1PORT 	B
#define K1BIT 	1

#define K2PORT	B
#define K2BIT 	2

#define K3PORT	B
#define K3BIT 	3

#define K4PORT	C
#define K4BIT 	0

#define K5PORT	C
#define K5BIT 	4

#define K6PORT	C
#define K6BIT 	5

//INPUT ONLY :
#define K7PORT	E
#define K7BIT 	3

#define K1AN 10
#define K2AN 8
#define K3AN 9


#endif
