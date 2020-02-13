/*********************************************************************
 *
 *                Fraise core
 *
 *********************************************************************

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) Antoine Rousseau   2009-2014   
 ********************************************************************/
 
#include "core.h"
#pragma config XINST=OFF // remove SDCC warning about XINST

//------------- time ---------------------------------------------
volatile unsigned int TMR0U=0; // upper word of TMR0
volatile DWORD Now;// time at last high interrupt

//------------- eeprom -------------------------------------------
void eeWriteByte(unsigned char address, unsigned char value){

    EEDATA = value;
    EEADR = address;
    // start write sequence as described in datasheet, page 91
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1; // enable writes to data EEPROM
    INTCONbits.GIEH = 0;  // disable interrupts
    INTCONbits.GIEL = 0;  // disable interrupts
    EECON2 = 0x55;
    EECON2 = 0x0AA;
    EECON1bits.WR = 1;   // start writing
    while(EECON1bits.WR){
        __asm nop __endasm;}
    if(EECON1bits.WRERR){
        //printf("ERROR: writing to EEPROM failed!\n");
    }
    EECON1bits.WREN = 0;
    INTCONbits.GIEH = 1;  // enable interrupts
    INTCONbits.GIEL = 1;  // enable interrupts
}

unsigned char eeReadByte(unsigned char address){
    EEADR = address;
    EECON1bits.CFGS = 0;
    EECON1bits.EEPGD = 0;
    EECON1bits.RD = 1;
    return EEDATA;
}

#if CONFIG_SETUP==1
extern void Setup();
#endif	

#ifndef FRAISE_NONE
extern void fraiseISR();
#endif

void coreInit(void)
{
#if CONFIG_SETUP==1
	Setup();
#endif	

	// Fake port init :
	PORTZ = LATZ = 0;
	PORTZbits.RZ1 = LATZbits.LATZ1 = 1;	

    // Master clock on Timer0 :
	T0CONbits.TMR0ON = 1;
	T0CONbits.T08BIT = 0;	// 16 bit timer
	T0CONbits.T0CS = 0;		// Use internal clock
	T0CONbits.T0SE = 1;		// Hi to low
	T0CONbits.PSA = 0;		// Use the prescaler
	T0CONbits.T0PS2 = 0;	// 
	T0CONbits.T0PS1 = 1;	// 1/16 prescaler: 
	T0CONbits.T0PS0 = 1;	// 
	INTCONbits.TMR0IE=1;	// enable TMR0 interrupt
	INTCON2bits.TMR0IP=1;	// high priority
	//T0CON=0b10010111;

	RCONbits.IPEN = 1;  // enable interrupts priority mode

	INTCONbits.GIEH = 1;  // enable interrupts
	INTCONbits.GIEL = 1;  // enable interrupts

// --------- init TMR2 as default PWM source -----------
	// fPWM = fOSC / (TMR2prescale * 4 * (PR2+1))
	//		= 64000000Hz/(4 * 4 * 256) = 15.625 kHz
	T2CONbits.T2CKPS0=1; //prescaler 4
	PR2=255;
	T2CONbits.TMR2ON=1;
}

//--------------------- Main : -------------------------------

void main() {
#ifdef UD_SETUP
	setup();
#endif

#ifdef UD_LOOP
	while(1) loop();
#endif
}

//--------------------- Interrupts : -------------------------------

//#define SAVETBLPRT() unsigned char savedTBLPTR = TBLPTR, savedTBLPTRL = TBLPTRL, savedTBLPTRH = TBLPTRH, savedTBLPTRU = TBLPTRU, savedTABLAT = TABLAT
//#define RESTORETBLPRT() TBLPTR = savedTBLPTR, TBLPTRL = savedTBLPTRL, TBLPTRH = savedTBLPTRH, TBLPTRU = savedTBLPTRU, TABLAT = savedTABLAT
#define SAVETBLPRT() POSTDEC1 = TBLPTR, POSTDEC1 = TBLPTRL, POSTDEC1 = TBLPTRH, POSTDEC1 = TBLPTRU, POSTDEC1 = TABLAT, POSTDEC1 = FSR1L, POSTDEC1 = FSR1H
#define RESTORETBLPRT() FSR1H = PREINC1, FSR1L = PREINC1, TABLAT = PREINC1, TBLPTRU = PREINC1, TBLPTRH = PREINC1, TBLPTRL = PREINC1, TBLPTR = PREINC1

void high_ISR(void)
//#ifdef SDCC
 //__shadowregs 
 __interrupt 1
//#endif
{
	SAVETBLPRT();
	
	Now.word1= TMR0U;
	LOWER_LSB(Now)=TMR0L;
	LOWER_MSB(Now)=TMR0H;
	
	if(INTCONbits.TMR0IF) {
		TMR0U++;
		INTCONbits.TMR0IF=0;
		Now.word1= TMR0U;
		LOWER_LSB(Now)=TMR0L;
		LOWER_MSB(Now)=TMR0H;
	}
	
#ifdef UD_HIGH
	highInterrupts();
#endif
	RESTORETBLPRT();
}



void low_ISR(void) 
//#ifdef SDCC
 __interrupt 2
//#endif
{	
	SAVETBLPRT();
	fraiseISR();
	
#ifdef UD_LOW
	lowInterrupts();
#endif
	RESTORETBLPRT();
}


//------------------------- Time : ----------------

unsigned unsigned long int time() 
{
	DWORD now;

	now.word1= TMR0U;
	
	LOWER_LSB(now)=TMR0L;
	LOWER_MSB(now)=TMR0H;
	
	if(now.word1 != TMR0U) { //a rollover occured : do read again
		now.word1= TMR0U;
		LOWER_LSB(now)=TMR0L;
		LOWER_MSB(now)=TMR0H;
	}	
		
	return now._dword;
}

//------------- fake port Z -------------------------------------

__at(_PORTZ_ADDR) volatile unsigned char PORTZ;
__at(_PORTZ_ADDR) volatile __PORTZbits_t PORTZbits;

__at(_PORTZ_ADDR + _PORT_TO_LAT) volatile unsigned char LATZ;
__at(_PORTZ_ADDR + _PORT_TO_LAT) volatile __LATZbits_t LATZbits;

__at(_PORTZ_ADDR + _PORT_TO_TRIS) volatile unsigned char TRICZ;
__at(_PORTZ_ADDR + _PORT_TO_TRIS) volatile __TRISZbits_t TRISZbits;

__at(_PORTZ_ADDR + _PORT_TO_ANSEL) volatile unsigned char ANSELZ;
__at(_PORTZ_ADDR + _PORT_TO_ANSEL) volatile __ANSELZbits_t ANSELZbits;


