/*********************************************************************
 *
 *                DMX library for Fraise pic18f device
 *
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
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

#include <dmx.h>


//serial port:
#if DMX_UART_NUM==1
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

unsigned char DMXRegisters[DMX_NBCHAN];
unsigned int DMXframeCount;

void Set250kB(void)
{
//baud rate : br=FOSC/[4 (n+1)] : n=FOSC/(4*br)-1 : br=250kHz, n=FOSC/1000000 - 1
#define BRGHL (FOSC / 1000000 - 1)
	SPBRGHx = BRGHL / 256;
	SPBRGx = BRGHL % 256;
}

void Set96kB(void)
{
//baud rate : br=FOSC/[4 (n+1)] : n=FOSC/(4*br)-1 : br=96kHz, n=FOSC/384000 - 1
#define BRGHL96 (FOSC/384000 - 1)
	SPBRGHx = BRGHL96 / 256;
	SPBRGx = BRGHL96 % 256;
}

void DMXInit(void)
{
	int i;
	
	for(i = 0; i < DMX_NBCHAN; i++) DMXRegisters[i] = 0; //clear all channels
	
	digitalSet(DMX_UART_PIN);
	pinModeDigitalOut(DMX_UART_PIN);

	BAUDCONxbits.BRG16 = 1;

	TXSTAxbits.TXEN = 1;
	TXSTAxbits.BRGH = 1;
	TXSTAxbits.TX9 = 1;
	TXSTAxbits.TX9D = 1;

	RCSTAxbits.SPEN = 1;
	
	DMXframeCount = 0;
}

void DMXSet(unsigned int channel, unsigned char value)
{
	if(channel == 0) return;
	DMXRegisters[channel] = value;
}

void DMXService()
{
	static int channel = -3;
	static int MABstartTime;
	
	if(!TXSTAxbits.TRMT) return;
	if(!TXxIF) return;
	
	if(channel == -3) { // BREAK
		Set96kB(); // 1 byte = 10 bits = 104 us
		TXREGx = 0;
		channel = -2;
	} else if(channel == -2) { // MARK AFTER BREAK start
		MABstartTime = time();
		channel = -1;
	} else if(channel == -1) { // MARK AFTER BREAK end
		if(elapsed(MABstartTime) > microToTime(8)) // MAB = 8 microseconds minimum
			channel = 0;
	} else if(channel == 0) {
		Set250kB();
		TXREGx = 0;
		channel = 1;
	} else {
		TXREGx = DMXRegisters[channel++];
		if(channel == DMX_NBCHAN) {
			channel = -3;
			DMXframeCount++;
		}
	}
}

