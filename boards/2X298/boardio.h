#ifndef _2X298_IO_
#define _2X298_IO_

#define MOTA_EN  			LATCbits.LATC2
#define MOTA_IN1 			LATAbits.LATA6
#define MOTA_IN2 			LATAbits.LATA7
#define SETPORT_MOTA do{ TRISCbits.TRISC2=TRISAbits.TRISA6=TRISAbits.TRISA7=0;} while(0)

#define MOTB_EN 			LATCbits.LATC1
#define MOTB_IN1 			LATAbits.LATA5
#define MOTB_IN2 			LATAbits.LATA4
#define SETPORT_MOTB do{ TRISCbits.TRISC1=TRISAbits.TRISA5=TRISAbits.TRISA4=0;} while(0)

#define MOTC_EN 			LATBbits.LATB5
#define MOTC_IN1 			LATBbits.LATB4
#define MOTC_IN2 			LATBbits.LATB3
#define SETPORT_MOTC do{ TRISBbits.TRISB5=TRISBbits.TRISB4=TRISBbits.TRISB3=0;} while(0)

#define MOTD_EN 			LATBbits.LATB0
#define MOTD_IN1 			LATBbits.LATB2
#define MOTD_IN2 			LATBbits.LATB1
#define SETPORT_MOTD do{ TRISBbits.TRISB0=TRISBbits.TRISB1=TRISBbits.TRISB2=0;} while(0)

#define K1PORT 	C
#define K1BIT 	4

#define K2PORT	C
#define K2BIT 	5

#define K3PORT	A
#define K3BIT 	0

#define K4PORT	A
#define K4BIT 	1

#define K5PORT	A
#define K5BIT 	2

#define K6PORT	A
#define K6BIT 	3

#define K8PORT	B
#define K8BIT 	6

#define K10PORT	B
#define K10BIT 	7

#define K11PORT	C
#define K11BIT 	3

#define MDEPORT B
#define MDEBIT 	0
//#define KPORT(num) K##num##PORT
//#define KBIT(num) K##num##BIT
#define MD1PORT B
#define MD1BIT 2

#define MD2PORT B
#define MD2BIT 1

#endif
