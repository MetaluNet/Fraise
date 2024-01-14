/*********************************************************************
 *
 *                DMX library for Fraise pic18f device
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  may 2012     Original.
 ********************************************************************/

/*
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
*/
#include <string.h>
#include <config.h>
#include <core.h>
#include <dmx_slave.h>

#ifndef DMX_SLAVE_UART_PORT
#define DMX_SLAVE_UART_PORT AUXSERIAL_NUM
#define DMX_SLAVE_UART_PIN AUXSERIAL_RX
#endif

//serial port:
#if DMX_SLAVE_UART_PORT==1
#define SPBRGx 			SPBRG1
#define SPBRGHx 		SPBRGH1
#define BAUDCONx 		BAUDCON1
#define BAUDCONxbits 	BAUDCON1bits
#define RCREGx 			RCREG1
#define RCSTAx 			RCSTA1
#define RCSTAxbits 		RCSTA1bits
#define TXREGx 			TXREG1
#define TXSTAx 			TXSTA1
#define TXSTAxbits 		TXSTA1bits
#define RCxIF	 		PIR1bits.RC1IF
#define TXxIF	 		PIR1bits.TX1IF
#define RCxIE	 		PIE1bits.RC1IE
#define TXxIE	 		PIE1bits.TX1IE
#define RCxIP	 		IPR1bits.RC1IP
#define TXxIP	 		IPR1bits.TX1IP
#else
#define SPBRGx 			SPBRG2
#define SPBRGHx 		SPBRGH2
#define BAUDCONx 		BAUDCON2
#define BAUDCONxbits 	BAUDCON2bits
#define RCREGx 			RCREG2
#define RCSTAx 			RCSTA2
#define RCSTAxbits 		RCSTA2bits
#define TXREGx 			TXREG2
#define TXSTAx 			TXSTA2
#define TXSTAxbits 		TXSTA2bits
#define RCxIF	 		PIR3bits.RC2IF
#define TXxIF	 		PIR3bits.TX2IF
#define RCxIE	 		PIE3bits.RC2IE
#define TXxIE	 		PIE3bits.TX2IE
#define RCxIP	 		IPR3bits.RC2IP
#define TXxIP	 		IPR3bits.TX2IP
#endif
//(1 port equ)
#ifndef BAUDCON1
#define SPBRG1 			SPBRG
#define SPBRGH1 		SPBRGH
#define BAUDCON1 		BAUDCON
#define BAUDCON1bits 	BAUDCONbits
#define RCREG1 			RCREG
#define RCSTA1 			RCSTA
#define RCSTA1bits 		RCSTAbits
#define TXREG1 			TXREG
#define TXSTA1 			TXSTA
#define TXSTA1bits 		TXSTAbits
#define RC1IF 			RCIF
#define TX1IF 			TXIF
#define RC1IE 			RCIE
#define TX1IE 			TXIE
#define RC1IP 			RCIP
#define TX1IP 			TXIP
#endif

#if DMX_SLAVE_NBCHAN > 256
static unsigned char DMXRegisters[256];
static unsigned char DMXRegistersB[DMX_SLAVE_NBCHAN - 256];
#else
static unsigned char DMXRegisters[DMX_SLAVE_NBCHAN];
#endif

#define STATE_NONE 0
#define STATE_BREAKRCVD 1
#define STATE_DMX 2
static unsigned char State = STATE_NONE;
static unsigned int DMXPointer = 0;
static unsigned int MaxReceivedChannel;

char DMXSlaveHadInterrupt = 0;

void Set250kB(void)
{
//baud rate : br=FOSC/[4 (n+1)] : n=FOSC/(4*br)-1 : br=250kHz, n=FOSC/1000000 - 1
#define BRGHL (FOSC/1000000 - 1)
	SPBRGHx=BRGHL/256;
	SPBRGx=BRGHL%256;
}

/*void Set96kB(void)
{
//baud rate : br=FOSC/[4 (n+1)] : n=FOSC/(4*br)-1 : br=96kHz, n=FOSC/384000 - 1
#define BRGHL96 (FOSC/384000 - 1)
	SPBRGHx=BRGHL96/256;
	SPBRGx=BRGHL96%256;
}*/

static void Serial_Init_Receiver() {
	WREG=RCREGx;
	__asm nop __endasm ;	
	WREG=RCREGx;
	RCSTAxbits.CREN = 0;
	RCSTAxbits.CREN = 1;
	RCSTAxbits.ADDEN = 0;
	RCxIE = 1;
	TXxIE = 0;
}

void DMXSlaveInit(void)
{
	int i;
	
	for(i=0;i<DMX_SLAVE_NBCHAN;i++) {
#if DMX_SLAVE_NBCHAN > 256
		if(i >= 256) DMXRegistersB[i - 256] = 0;
		else
#endif
			DMXRegisters[i]=0; //clear all channels
	}

	pinModeDigitalIn(DMX_SLAVE_UART_PIN);
	BAUDCONxbits.BRG16=1;
	Set250kB();
	Serial_Init_Receiver();
	//TXSTAxbits.TXEN=1;
	TXSTAxbits.BRGH=1;
	RCSTAxbits.RX9 = 1;

	RCxIP = 0; // low interrupt priority

	RCSTAxbits.SPEN=1;
}

unsigned char DMXSlaveGet(unsigned int channel)
{
	if(channel==0) return 0;
	if(channel >= DMX_SLAVE_NBCHAN) return 0;
#if DMX_SLAVE_NBCHAN > 256
	if(channel >= 256) return DMXRegistersB[channel - 256];
	else
#endif
	return DMXRegisters[channel];
}

void DMXSlaveISR()
{
	unsigned char Byte;
	
	if(!RCxIF) return;
			
	if(RCSTAxbits.OERR){ // overrun error
		State=STATE_NONE;
		//Byte=RCREG;Byte=RCREG;
		WREG=RCREGx;
		__asm nop __endasm ;	
		WREG=RCREGx;
		RCSTAxbits.CREN=0;
		RCSTAxbits.CREN=1;
		return;
	}
	if(RCSTAxbits.FERR){ // framing error -> interpreted as a break
		
		/*if(State==STATE_BREAKRCVD) {
			DMXSlaveHadInterrupt = 1;
			if(TXxIF) TXREGx = 0b01010101;
		}*/
		State=STATE_BREAKRCVD;
		WREG=RCREGx;
		//RCSTAxbits.CREN=0;
	//while(!digitalRead(DMX_SLAVE_UART_PIN));	
		//RCSTAxbits.CREN=1;
		//if(TXxIF) TXREGx = 0;
		//WREG=RCREGx;
		/*WREG=RCREGx;
		__asm nop __endasm ;	
		WREG=RCREGx;*/
		/*RCSTAxbits.CREN=0;
		RCSTAxbits.SPEN=0;
		RCSTAxbits.SPEN=1;
		RCSTAxbits.CREN=1;*/
		/*WREG=RCREGx;
		__asm nop __endasm ;	
		WREG=RCREGx;
		RCSTAxbits.CREN=0;
		RCSTAxbits.CREN=1;*/
		return;
	}
	if(!RCSTAxbits.RX9D){
		State=STATE_NONE;
		WREG=RCREGx;
		return;
	}

	Byte = RCREGx;
	
	
	if(State==STATE_BREAKRCVD){
		
		if(Byte==0){
			State=STATE_DMX;
			DMXPointer=0;
			if(TXxIF) TXREGx = 1;
			DMXSlaveHadInterrupt = 1;
		}
		else {
			State=STATE_NONE;
			if(TXxIF) TXREGx = 255;
		}
		return;
	}
	
	if(State==STATE_DMX){
		DMXPointer++;
		if(MaxReceivedChannel < DMXPointer) MaxReceivedChannel = DMXPointer;
		if(DMXPointer < DMX_SLAVE_NBCHAN) {
#if DMX_SLAVE_NBCHAN > 256
			if(DMXPointer >= 256) DMXRegistersB[DMXPointer - 256] = Byte;
			else
#endif
			DMXRegisters[DMXPointer] = Byte;
		}
	}
	//DMXSlaveHadInterrupt = 1;
}

unsigned int DMXSlaveGetMaxReceivedChannel()
{
	unsigned int tmp = MaxReceivedChannel;

	RCxIE = 0; //disable RX interrupt
	MaxReceivedChannel = 0;
	RCxIE = 1;//enable RX interrupt
	
	return tmp;
}

void DMXSlaveSet(unsigned int channel, unsigned char value)
{
	if((channel<=0) || (channel >= DMX_SLAVE_NBCHAN)) return;
#if DMX_SLAVE_NBCHAN > 256
	if(channel >= 256) DMXRegistersB[channel - 256] = value;
	else
#endif
		DMXRegisters[channel] = value;
}

