
#include <core.h>
#include "spimaster.h"

//CKE=1 CKP=0 SMP=0	
#define SPImasterXInit(X) do {\
		SSP##X##STATbits.SMP = 0; \
		SSP##X##STATbits.CKE = 1; \
		SSP##X##CON1.CKP = 0; \
		SSP##X##CON1.SSP##X##EN = 1; \
		SSP##X##CON1.SSP##X##M = 0b0001; /* SPI Master mode, clock = FOSC/16 */ \
	} while(0);
	

void SPImaster1Init()
{
#ifdef _PORTC_SDO1
	TRISC &= ~_PORTC_SDO1;
#endif

#ifdef _PORTC_SCK1
	TRISC &= ~_PORTC_SCK1;
#endif

#ifdef _PORTC_SDI1
	TRISC |= _PORTC_SDI1;
#endif

	SPImasterXInit(1);
}

void SPImaster2Init()
{
#ifdef _PORTB_SDO2
	TRISB &= ~_PORTB_SDO2;
#else 
#ifdef _PORTD_SDO2
	TRISD &= ~_PORTD_SDO2;
#endif

#ifdef _PORTB_SCK2
	TRISB &= ~_PORTB_SCK2;
#else
#ifdef _PORTD_SCK2
	TRISD &= ~_PORTD_SCK2;	
#endif

#ifdef _PORTB_SDI2
	TRISB |= _PORTB_SDI2;
#else
#ifdef _PORTD_SDI2
	TRISD |= _PORTD_SDI2;
#endif

	SPImasterXInit(2);
}


byte SPImaster1Transfer(byte b)
{
	SSP1BUF = b;
	while(!SSP1STATbits.BF);
	return SSP1BUF;
}

byte SPImaster2Transfer(byte b);
{
	SSP2BUF = b;
	while(!SSP2STATbits.BF);
	return SSP2BUF;
}


