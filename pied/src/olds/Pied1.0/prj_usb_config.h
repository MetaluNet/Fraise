#ifndef USB_HARDWARE_PROFILE_H
#define USB_HARDWARE_PROFILE_H

#if 0
#include "config.h"
//#define CLOCK_FREQ 48000000
//#define BAUDCLOCK_FREQ 12000000 // (48000000 /4) required for baud rate calculations
//#define UART_BAUD_setup(x)  SPBRG = x & 0xFFu; SPBRGH = (x >> 8) & 0xFFu
//#define CDC_FLUSH_MS 4 // how many ms timeout before cdc in to host is sent

#define USB_VID (0x4d8U)
#define USB_PID (0x000aU)  // Microchip CDC  
#define USB_DEV 0x0002U

#define USB_NUM_CONFIGURATIONS          1u
#define USB_NUM_INTERFACES              2u
#define USB_NUM_ENDPOINTS               3u


#define MAX_EPNUM_USED                  2u

#define USB_BUS_POWERED 1
#define USB_INTERNAL_TRANSCIEVER 1
#define USB_INTERNAL_PULLUPS 1
#define USB_INTERNAL_VREG 1
#define USB_FULL_SPEED_DEVICE 1

/* PingPong Buffer Mode
 * Valid values
 * 0 - No PingPong Buffers
 * 1 - PingPong on EP0
 * 2 - PingPong on all EP
 * 3 - PingPong on all except EP0
 */
#define USB_PP_BUF_MODE 0
#define USB_EP0_BUFFER_SIZE 8u
#define CDC_NOTICE_BUFFER_SIZE 10u

/* Low Power Request
 * Optional user supplied subroutine to set the circuit
 * into low power mode during usb suspend.
 * Probably needed when bus powered as only 2.5mA should
 * be drawn from bus i suspend mode */
//#define usb_low_power_request() Nop()

#endif

/*Choose your hardware configuration
*
*  Defaults are:
*	PIC18F2550 - PiedUsb
*/
#define PIEDUSB

#ifdef PIEDUSB
//	#define __18F2550
//	#define __18F25K50
//	#define CDC_BUFFER_SIZE 64u
	
//	#define CLOCK_FREQ 48000000
//	#define BAUDCLOCK_FREQ 12000000 // (48000000 /4) required for baud rate calculations
//	#define UART_BAUD_setup(x) // SPBRG = x & 0xFFu; SPBRGH = (x >> 8) & 0xFFu
//	#define CDC_FLUSH_MS 4 // how many ms timeout before cdc in to host is sent
	
//	#define USB_INTERRUPTS //use interrupts instead of polling

	/** L E D ***********************************************************/
	/* On UBW, LED1 = RC0, LED2 = RC1, SW = RC2		 					*/


	#define mLED_1              LATAbits.LATA6
	#define mLED_2              LATCbits.LATC1

	#define mLED_1_On()         do{mLED_1 = 1;} while(0)
	#define mLED_2_On()         do{mLED_2 = 1;} while(0)

	#define mLED_1_Off()        do{mLED_1 = 0;} while(0)
	#define mLED_2_Off()        do{mLED_2 = 0;} while(0)

	#define mLED_1_Toggle()     do{mLED_1 = !mLED_1;} while(0)
	#define mLED_2_Toggle()     do{mLED_2 = !mLED_2;} while(0)

	#define mInitAllLEDs()      do{TRISAbits.TRISA6 = TRISCbits.TRISC1 = 0; mLED_1_Off(); mLED_2_Off();} while(0)

	/** S W I T C H *****************************************************/
	//#define mInitSwitch()		TRISCbits.TRISC2 = 1;
	//#define UserSW				PORTCbits.RC2

	/** SERIAL DRIVER *****************************************************/
	#define	mSerDrv	TRISCbits.TRISC0
	#define	mInitSerDrv()	do{TRISCbits.TRISC0 = 1; LATCbits.LATC0 = 0;} while(0)
	#define	mSerDrv_On()	do{mSerDrv=0;} while(0)
	#define	mSerDrv_Off()	do{mSerDrv=1;} while(0)
	#define	mSerDrv_isOn()	(mSerDrv==0)

	
#endif //end processor setup


#endif
