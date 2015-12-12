#ifndef _ILOTOPONTV2_IO_
#define _ILOTOPONTV2_IO_

#define MOT_A  				LATCbits.LATC1
#define MOT_B  				LATCbits.LATC0
#define MOT_C  				LATAbits.LATA6
#define MOT_D  				LATAbits.LATA7

#define MOT_DIS  			LATAbits.LATA5

#define SETPORT_MOTOR do{ 	TRISCbits.TRISC1=TRISCbits.TRISC0=TRISAbits.TRISA6=TRISAbits.TRISA7=TRISAbits.TRISA5=0;\
								 \
						} while(0)
#define MOT_PWM		2 		// motor pwm number

#define LAMP1 				LATBbits.LATB0
#define LAMP2 				LATBbits.LATB4
#define LAMP3 				LATBbits.LATB5
#define LAMP4 				LATAbits.LATA4
#define SETPORT_LAMPS do{ TRISBbits.TRISB0=TRISBbits.TRISB4=TRISBbits.TRISB5=TRISAbits.TRISA4=0;\
							PSTR1CON=0; PSTR1CONbits.STR1D=1; } while(0)

#define 	LAMP1_PWM	4
#define 	LAMP2_PWM	1
#define 	LAMP3_PWM	3
#define 	LAMP4_PWM	5

#define K1PORT 	B
#define K1BIT 	1

#define K2PORT	B
#define K2BIT 	3

#define K3PORT	B
#define K3BIT 	2

#define CURRENT_AN	0 		// current measure
#define TEMP_AN 	1		// temperature analog channel
#define VBATT_AN		2		// battery voltage analog channel

#define SERIAL_DRV	LATCbits.LATC5 // aux serial port
#define SETPORT_SERIAL do { TRISCbits.TRISC5=TRISCbits.TRISC6=0; } while(0)

#endif
