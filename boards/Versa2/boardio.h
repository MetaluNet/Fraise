#ifndef _VERSA2_IO_
#define _VERSA2_IO_

#define K1PORT 	A
#define K1BIT 	0
#define K1AN	0

#define K2PORT	A
#define K2BIT 	1
#define K2AN	1

#define K3PORT	A
#define K3BIT 	2
#define K3AN	2

#define K4PORT	A
#define K4BIT 	3
#define K4AN 	3
//#define K4PWM	5
//#define K4SETUP_PWM() do{} while(0)

#define K5PORT	C
#define K5BIT 	3
#define K5AN	15

#define K6PORT	C
#define K6BIT 	2
#define K6AN	14
#define K6PWM	1
#define K6SETUP_PWM() do{PSTR1CON=0; PSTR1CONbits.STR1A=1;} while(0)

#define K7PORT	C
#define K7BIT 	4
#define K7AN	16

#define K8PORT	C
#define K8BIT 	5
#define K8AN	17

#define K9PORT	B
#define K9BIT 	2
#define K9AN	8
#define K9INT	2
#define K9PWM	1
#define K9SETUP_PWM() do{PSTR1CON=0; PSTR1CONbits.STR1B=1;} while(0)

#define K10PORT	B
#define K10BIT 	3
#define K10AN	9
#define K10PWM	2
#define K10SETUP_PWM() do{PSTR2CON=0; PSTR2CONbits.STR2A=1;} while(0)

#define K11PORT	B
#define K11BIT 	6

#define K12PORT	B
#define K12BIT 	7

#define LEDPORT	C
#define LEDBIT 	1

//---- MOTOR A
#define MA1PORT	A
#define MA1BIT	5
#define MA1AN	4

#define MA2PORT	A
#define MA2BIT	7

#define MAENPORT A
#define MAENBIT  6

#define MAEN2   K2

#define MOTA_PWM 5
#define MAPWMPORT A
#define MAPWMBIT 4
#define MAPWMAN 5
#define MAPWMPWM 5
#define MAPWMSETUP_PWM() do{} while(0)

#define MOTA_CURRENT K3

#define MOTA_CONFIG() do{/*set PWM pin digi out :*/pinModeDigitalOut(MAPWM);} while(0)

//---- MOTOR B
#define MB1PORT	B
#define MB1BIT	0
#define MB1AN	12
#define MB1INT	0
#define MB1PWM	4
#define MB1SETUP_PWM() do{} while(0)

#define MB2PORT	B
#define MB2BIT	5
#define MB2AN	13
#define MB2PWM	3
#define MB2SETUP_PWM() do{} while(0)

#define MBENPORT B
#define MBENBIT 1
#define MBENAN	10
#define MBENINT	1

#define MBEN2PORT B
#define MBEN2BIT 4
#define MBEN2AN	11

#define MOTB_PWM 1
#define MBPWM K9
#define MOTB_CURRENT K10

#define MOTB_CONFIG() do{/*init PWM1 to P1B for MOTB_PWM : */ PSTR1CON=0; PSTR1CONbits.STR1B=1;pinModeDigitalOut(K9);} while(0)


//---- Aux serial port
#define AUXSERIAL_NUM 2
#define AUXSERIAL_TX K11
#define AUXSERIAL_RX K12

//---- I2C
#define I2C1SCL K5
#define I2C1SDA K7

#define I2C2SCL MBEN
#define I2C2SDA K9

//---- SPI
#define SPI1SCK K5
#define SPI1SDI K7
#define SPI1SDO K8

#define SPI2SCK MBEN
#define SPI2SDI K9
#define SPI2SDO K10

#endif // ifndef _VERSA2_IO_
