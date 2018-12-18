/*-------------------------------------------------------------------------
  config.c - PIC configuration words

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

/* Generic Setup() */
void Generic26kSetup()
{
	OSCCON=0b01110000;
	OSCTUNEbits.PLLEN=1;
#if UART_PORT==1
	ANSELCbits.ANSC7=0; //enable digital input buffer for RX pin
#endif
	TRISA=0xff;
	TRISB=0xff;
	TRISC=0xff;
	TRISE=0xff;
	IOCB=0; //disable all 4 portB "interrupt on change"
	INTCON2bits.RBPU=0; //enable PORTB weak pullups
}

#ifdef BOOTLOADER
/* Set the PIC config words */
__code char __at __CONFIG1H _conf1 = _FOSC_INTIO67_1H 	// Internal oscillator block
								&	_PLLCFG_ON_1H 		// Oscillator multiplied by 4
								&	_PRICLKEN_ON_1H 	// Primary clock enabled
								&	_FCMEN_OFF_1H 		// Fail-Safe Clock Monitor disabled
								&	_IESO_OFF_1H;		// Oscillator Switchover mode disabled

__code char __at __CONFIG2L _conf2 = _PWRTEN_ON_2L		// Power up timer enabled
								&	_BOREN_ON_2L		// Brown-out Reset enabled and controlled by software (SBOREN is enabled)
								&	_BORV_190_2L;		// VBOR set to 1.9 V nominal

__code char __at __CONFIG2H _conf3 = _WDTEN_SWON_2H		// WDT is controlled by SWDTEN bit of the WDTCON register
								&	_WDTPS_256_2H;		// 1:256 = 1.024s

__code char __at __CONFIG4L _conf5 =	//_STVREN_OFF_4L	// Stack full/underflow will not cause Reset
									_STVREN_ON_4L		// Stack full/underflow will cause Reset
								&	_LVP_OFF_4L			// Single-Supply ICSP disabled
								&	_XINST_OFF_4L		// Instruction set extension and Indexed Addressing mode disabled (Legacy mode)
								&	_DEBUG_OFF_4L;		// Disabled

__code char __at __CONFIG5L _conf6 = _CP0_OFF_5L		// Block 0 (000800-003FFFh) not code-protected
								&	_CP1_OFF_5L			// Block 1 (004000-007FFFh) not code-protected
								&	_CP2_OFF_5L			// Block 2 (008000-00BFFFh) not code-protected
								&	_CP3_OFF_5L	;		// Block 3 (00C000-00FFFFh) not code-protected

__code char __at __CONFIG5H _conf7 = _CPB_OFF_5H		// Boot block (000000-0007FFh) not code-protected
                                &	_CPD_OFF_5H;		// Data EEPROM not code-protected

__code char __at __CONFIG6L _conf8 = 0xff;				// no code protection at all

__code char __at __CONFIG6H _conf9 = 0xff;

__code char __at __CONFIG7L _conf10 = 0xff;

__code char __at __CONFIG7H _conf11 = 0xff;

#endif //BOOTLOADER


