#ifndef _STEP1_2_IO_
#define _STEP1_2_IO_

#define MOTA_EN  			LATBbits.LATB3
#define MOTA_IN1 			LATAbits.LATA4
#define MOTA_IN2 			LATBbits.LATB5
#define SETPORT_MOTA do{ TRISAbits.TRISA4=TRISBbits.TRISB3=TRISBbits.TRISB5=0;} while(0)

#define MOTB_EN 			LATBbits.LATB4
#define MOTB_IN1 			LATBbits.LATB0
#define MOTB_IN2 			LATBbits.LATB1
#define SETPORT_MOTB do{ TRISBbits.TRISB4=TRISBbits.TRISB0=TRISBbits.TRISB1=0;} while(0)

#define SENSE_APORT A
#define SENSE_ABIT 	0
#define SENSE_AAN 	0

#define SENSE_BPORT A
#define SENSE_BBIT 	1
#define SENSE_BAN 	1

#define LEDPORT A
#define LEDBIT 	7

#define K1PORT 	A
#define K1BIT 	5
#define K1AN	4

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
#define K7BIT 	2
#define K7AN	8


#endif
