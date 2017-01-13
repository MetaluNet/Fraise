/* 
This file is licensed under the MIT license:

Copyright (c) 2010,2013 Kustaa Nyholm / SpareTimeLabs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

 Version 1.1     Compatible with SDCC 3.x

 */


#include "pic18f2550.h"
#include "usbcdc.h"
#include "usb_defs.h"
#include "pic-config.c"
#include "printft.h"


void high_isr(void) __shadowregs __interrupt 1
{
	if(PIR2bits.USBIF)
	{
		usbcdc_handler();
		PIR2bits.USBIF=0;
	}
}

void low_isr(void) __shadowregs __interrupt 2
{
	;
}


void putchar(char c) __wparam
{
	/*if (c=='\n') {
		usbcdc_putchar('\r');
		}*/

	usbcdc_putchar(c);
	if (c=='\n')
		usbcdc_flush();
}

char getchar() {
	//usbcdc_flush();
	return usbcdc_getchar();
}

void main(void) {
	OSCCON = 0x70;

	usbcdc_init();

	INTCONbits.PEIE = 1;
	INTCONbits.GIE = 1;

	while (usbcdc_device_state != CONFIGURED)
		;

	printft("Wellcome!\n");


	while (1) {
		//printft("%02d\n",getchar());
		//printft("%02d\n",getchar());
		//putchar(getchar());
		printft("%c",getchar());
	}

}
