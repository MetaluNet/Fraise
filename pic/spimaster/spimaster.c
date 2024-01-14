/*********************************************************************
 *
 *                SPI master library for Fraise pic18f  device
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  nov 2018   (nearly) initial
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

#include <core.h>
#include <spimaster.h>


//CKE=1 CKP=0 SMP=0	
#define _SPImasterInit(X) do {\
		SSP##X##STATbits.SMP = 1/*0*/; \
		SSP##X##STATbits.CKE = 1; \
		SSP##X##CON1bits.CKP = 0; \
		SSP##X##CON1bits.SSPEN = 1; \
		SSP##X##CON1bits.SSPM = SPI_BITRATE; /* SPI Master mode, clock = FOSC/16 */ \
		/*SSP##X##CON1bits.SSPM = 0b0010;*/ /* SPI Master mode, clock = FOSC/64 */ \
		pinModeDigitalOut(SPI##X##SCK); \
		pinModeDigitalIn(SPI##X##SDI); \
		pinModeDigitalOut(SPI##X##SDO); \
	} while(0);
	
#define SPImasterInit(X) _SPImasterInit(X)

void SPImaster1Init()
{
/*#ifdef _PORTC_SDO1
	TRISC &= ~_PORTC_SDO1;
#endif

#ifdef _PORTC_SCK1
	TRISC &= ~_PORTC_SCK1;
#endif

#ifdef _PORTC_SDI1
	TRISC |= _PORTC_SDI1;
#endif*/

	SPImasterInit(1);
}

void SPImaster2Init()
{
/*#ifdef _PORTB_SDO2
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
#endif*/

	SPImasterInit(2);
}

byte SPImaster1Transfer(byte b)
{
	SSP1BUF = b;
	while(!SSP1STATbits.BF);
	return SSP1BUF;
}

byte SPImaster2Transfer(byte b)
{
	SSP2BUF = b;
	while(!SSP2STATbits.BF);
	return SSP2BUF;
}


