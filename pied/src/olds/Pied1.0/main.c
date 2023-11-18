// Open source PIC USB stack echo demo
// USB stack by JTR and Honken
// CC-BY
//
// USB driver files should be in '..\dp_usb\'
// Enter a USB VID and PID in prj_usb_config.h

#include "prj_usb_config.h"
//USB stack
#include "usbcdc.h"
#include "usb_defs.h"
//#include "pic-config.c"
#include "18f25k50-config.h"

//FRAISE stack
#include "fraisemaster.h"
#include "stdio.h"

#include <pic18fregs.h>

#define BYTE unsigned char

//board hardware setup
//add your hardware here
void SetupBoard(void) {
#if defined (PIEDUSB)
    //disable some defaults
    //ADCON1 |= 0b1111; //all pins digital
    //CVRCON = 0b00000000;
    LATC = 0x00;
    TRISC = 0xFF;
#endif

// 18k25k50 specific: ----------------------
	OSCCON2bits.PLLEN = 0;    // Changes in SPLLMULT are ignored unless pll is disabled          
	OSCTUNEbits.SPLLMULT = 1; // PLL: x3 multiplier  Set this before enabling PLL          

	OSCCON = 0x70;          
	//OSCCONbits.SCS = 0;     //Primary clock determined by FOSC<3:0> in CONFIG1H          
	//OSCCONbits.IRCF = 7;    // Internal RC Oscillator HFINTOSC = 16 MHz          
	//OSCCONbits.IDLEN = 0;   // Device enters Sleep mode on SLEEP instruction          

	OSCCON2 = 0x10;          
	//OSCCON2bits.INTSRC = 0; // Low power INTRC as 31.25 reference          
	//OSCCON2bits.SOSCGO = 0; // No secondary oscillator unless otherewise requested          
	//OSCCON2bits.PRISD = 0;  // Enable primary oscillator          
	//OSCCON2bits.PLLEN = 1;  // Use PLL.          

	while (OSCCON2bits.PLLRDY != 1)          
	 ;          

	OSCCON2bits.PRISD = 1; // Disable primary oscillator          
	// The following is "probably" not necessary since not doing clock switching          
	//Delay10KTCYx(3);  // 2.5 milliseconds for 12 MHz instruction clock        

	// Enable HFINTOSC to be tuned by the USB clock          
	ACTCON = 0x90;          
	//ACTCONbits.ACTEN = 1;        
	//ACTCONbits.ACTSRC = 1;   
//---------------------------------

	// Initalize switchs and leds
    mInitAllLEDs();
    //mInitSwitch();

    mInitSerDrv();
    
    // disable analog for USART pins
    ANSELCbits.ANSC6 = 0;
    ANSELCbits.ANSC7 = 0;
}
//void USBSuspend(void);

//void _user_putc(char c)
#ifndef PUTCHAR
#define PUTCHAR(c) void putchar(char c) __wparam
#endif

PUTCHAR(c)
{
   //putc_cdc(c);
   	usbcdc_putchar(c);
	if (c=='\n')
		usbcdc_flush();

}

/*char getchar(BYTE ) {
	//usbcdc_flush();
	return usbcdc_getchar();
}*/

/*char pollchar() {
	if(!usbcdc_rd_ready()) return 0;*/

void main(void)
{
    BYTE RecvdByte;
    
	SetupBoard();
	mLED_1_Off();
	mLED_2_Off();

	//while(1);
    // Use our own special output function for STDOUT:
    //stdout = _H_USER; 
	stdout = STREAM_USER;
//    initCDC(); // setup the CDC state machine


	usbcdc_init();
	mLED_1_On();

//	mLED_2_Off();

	RCONbits.IPEN = 1;
	IPR3bits.USBIP = 0;
	INTCONbits.GIEL = 1;
	INTCONbits.GIEH = 1;

	while (usbcdc_device_state != CONFIGURED)

    
	mLED_2_On();
    
    FraiseInit();

// Main echo loop
    do {

// If USB_INTERRUPT is not defined each loop should have at least one additional call to the usb handler to allow for control transfers.
//#ifndef USB_INTERRUPTS
//        usb_handler();
//#endif

// Receive and send method 1
// The CDC module will call usb_handler each time a BULK CDC packet is sent or received.
// If there is a byte ready will return with the number of bytes available and received byte in RecvdByte
        /*if (poll_getc_cdc(&RecvdByte))
            putc_cdc(RecvdByte);*/
        //putchar(getchar());
            
        if(!FrGotLineFromUsb) {
            while(usbcdc_rd_ready()) {
                RecvdByte = usbcdc_getchar();
                if(RecvdByte=='\n') {
                    FrGotLineFromUsb=1;
                    //printf((const far rom char*)"rcvd line !\n");
                    break;
                }
                else if(LineFromUsbLen<(sizeof(LineFromUsb)-1))
                    LineFromUsb[LineFromUsbLen++]=RecvdByte;
            }
        }

        if(UIRbits.SOFIF==1) { FraiseSOF(); UIRbits.SOFIF=0; }

        FraiseService();


    } while (1);

} //end main



void InterruptHandlerLow(void) __interrupt 2
{
//    usb_handler();
//    ClearGlobalUsbInterruptFlag();
//    FraiseISR();
	if(PIR3bits.USBIF)
	{
		usbcdc_handler();
		PIR3bits.USBIF=0;
	}
}


void InterruptHandlerHigh(void) __shadowregs __interrupt 1
{ //Also legacy mode interrupt.
    //usb_handler();
    //ClearGlobalUsbInterruptFlag();
    FraiseISR();
    /*if(PIR2bits.USBIF)
	{
		usbcdc_handler();
		PIR2bits.USBIF=0;
	}*/

}


