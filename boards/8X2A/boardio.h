#ifndef _8X2A_IO_
#define _8X2A_IO_

/*#define MOTA_EN  			LATAbits.LATA4
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
#define SETPORT_MOTD do{ TRISCbits.TRISC0=TRISBbits.TRISB3=TRISAbits.TRISA7=0;} while(0)*/

// CONNECTORS :
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
#define K6PWM	3
#define K6SETUP_PWM() do{PSTR3CON=0; PSTR3CONbits.STR3B=1;} while(0)

#define K7PORT	B
#define K7BIT 	0
#define K7AN	12
#define K7INT   0
#define K7PWM	4
#define K7SETUP_PWM() do{} while(0)

#define K8PORT	B
#define K8BIT 	1
#define K8AN	10
#define K8INT   1
#define K8PWM	1
#define K8SETUP_PWM() do{PSTR1CON=0; PSTR1CONbits.STR1C=1;} while(0)

#define K9PORT	B
#define K9BIT 	2
#define K9AN	8
#define K9INT   2

// MOTOR A :
#define MA1PORT	A
#define MA1BIT	3
#define MA1AN	3

#define MA2PORT	A
#define MA2BIT	2
#define MA2AN	2

#define MAENPORT A
#define MAENBIT 4

#define MAEN2 KZ2

#define MOTA_PWM 5
#define MOTA_CONFIG() do{}while(0)

// MOTOR B :
#define MB1PORT	A
#define MB1BIT	1
#define MB1AN	1

#define MB2PORT	A
#define MB2BIT	0
#define MB2AN	0

#define MBENPORT B
#define MBENBIT 5

#define MBEN2 KZ2

#define MOTB_PWM 3
#define MOTB_CONFIG() do{}while(0)

// MOTOR C :
#define MC1PORT	B
#define MC1BIT	4
#define MC1AN	11

#define MC2PORT	A
#define MC2BIT	6

#define MCENPORT C
#define MCENBIT 2

#define MCEN2 KZ2

#define MOTC_PWM 1
#define MOTC_CONFIG() do{\
    /* init PWM1 to pulse MOTC_IN1 (P1D) : */ PSTR1CON=0; PSTR1CONbits.STR1D=1; \
}while(0)
 
// MOTOR D :
#define MD1PORT	B
#define MD1BIT	3
#define MD1AN	9

#define MD2PORT	A
#define MD2BIT	7

#define MDENPORT C
#define MDENBIT 0

#define MDEN2 KZ2

#define MOTD_PWM 2
#define MOTD_CONFIG() do{}while(0)
 
//---- Aux serial port :
#define AUXSERIAL_NUM 1
#define AUXSERIAL_TX K5
#define AUXSERIAL_RX K6

/*----------- I2C --------------*/
#define I2C1SCL K2
#define I2C1SDA K3

#define I2C2SCL K8
#define I2C2SDA K9

#endif
