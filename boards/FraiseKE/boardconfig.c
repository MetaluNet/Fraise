/*-------------------------------------------------------------------------
  boardconfig.c - hardware setup and PIC configuration words

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
-------------------------------------------------------------------------*/

#include <pic18fregs.h>
#include "../generic26kconfig.c"
/* Specific Setup() */
void Setup()
{
	Generic26kSetup();
}

#ifdef BOOTLOADER
/* Set the PIC config words */

__code char __at __CONFIG3H _conf4 = //_CCP2MX_PORTB3_3H	// CCP2 input/output is multiplexed with RB3
								 	_CCP2MX_PORTC1_3H	// CCP2 input/output is multiplexed with RC1
								//&	_PBADEN_OFF_3H			// PORTB<5:0> pins are configured as digital I/O on Reset
								&	_PBADEN_ON_3H		// PORTB<5:0> pins are configured as analog input channels on Reset
								//&	_CCP3MX_PORTC6_3H		// P3A/CCP3 input/output is mulitplexed with RC6
								&	_CCP3MX_PORTB5_3H	// P3A/CCP3 input/output is multiplexed with RB5
								&	_HFOFST_OFF_3H			// HFINTOSC output and ready status are delayed by the oscillator stable status
								//&	_HFOFST_ON_3H		// HFINTOSC output and ready status are not delayed by the oscillator stable status
								//&	_T3CMX_PORTB5_3H	// T3CKI is on RB5
								&	_T3CMX_PORTC0_3H		// T3CKI is on RC0
								&	_P2BMX_PORTC0_3H		// P2B is on RC0
								//&	_P2BMX_PORTB5_3H	// P2B is on RB5
								&	_MCLRE_INTMCLR_3H;	// RE3 input pin enabled; MCLR disabled
								//&	_MCLRE_EXTMCLR_3H		// MCLR pin enabled, RE3 input pin disabled

#endif //BOOTLOADER


