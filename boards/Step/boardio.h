#ifndef _STEP_IO_
#define _STEP_IO_

#define MOTA_EN  			LATAbits.LATA4
#define MOTA_IN1 			LATCbits.LATC0
#define MOTA_IN2 			LATCbits.LATC3
#define SETPORT_MOTA do{ TRISAbits.TRISA4=TRISCbits.TRISC0=TRISCbits.TRISC3=0;} while(0)

#define MOTB_EN 			LATAbits.LATA5
#define MOTB_IN1 			LATAbits.LATA7
#define MOTB_IN2 			LATAbits.LATA6
#define SETPORT_MOTB do{ TRISAbits.TRISA5=TRISAbits.TRISA7=TRISAbits.TRISA6=0;} while(0)

#define K1PORT 	E
#define K1BIT 	3

#define K2PORT	B
#define K2BIT 	6

#define K3PORT	B
#define K3BIT 	7

#define K4PORT	B
#define K4BIT 	5
#define K4AN	13

#define K5PORT	C
#define K5BIT 	4
#define K5AN	11

#define K6PORT	B
#define K6BIT 	0
#define K6AN	12

#define K7PORT	C
#define K7BIT 	7
#define K7AN	16

#define K8PORT	B // on ICSP
#define K8BIT 	1
#define K8AN	10

#define K9PORT	B // on ICSP
#define K9BIT 	2
#define K9AN	8

#define K10PORT	B // on ICSP
#define K10BIT 	3
#define K10AN	9

#define MA1PORT	C // MOTA_IN1
#define MA1BIT 	0

#define MA2PORT	C // MOTA_IN2
#define MA2BIT 	3
#define MA2AN	15

#define MB1PORT	A // MOTB_IN1
#define MB1BIT 	7

#define MB2PORT	A // MOTB_IN2
#define MB2BIT 	6

/*#define KPORT(k) k##PORT
#define KBIT(k)  k##BIT
#define KAN(k)  k##AN*/



#endif
