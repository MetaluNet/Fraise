#ifndef USB_HARDWARE_PROFILE_H
#define USB_HARDWARE_PROFILE_H

	/** L E D ***********************************************************/
	/* LED1 = RC2, LED2 = RC1 */


	#define mLED_1              LATCbits.LATC2
	#define mLED_2              LATCbits.LATC1

	#define mLED_1_On()         do{mLED_1 = 1;} while(0)
	#define mLED_2_On()         do{mLED_2 = 1;} while(0)

	#define mLED_1_Off()        do{mLED_1 = 0;} while(0)
	#define mLED_2_Off()        do{mLED_2 = 0;} while(0)

	#define mLED_1_Toggle()     do{mLED_1 = !mLED_1;} while(0)
	#define mLED_2_Toggle()     do{mLED_2 = !mLED_2;} while(0)

	#define mInitAllLEDs()      do{TRISCbits.TRISC2 = TRISCbits.TRISC1 = 0; mLED_1_Off(); mLED_2_Off();} while(0)

	/** SERIAL DRIVER *****************************************************/
	/* DRV = RC0 */
	#define	mSerDrv	TRISCbits.TRISC0
	#define	mInitSerDrv()	do{TRISCbits.TRISC0 = 1; LATCbits.LATC0 = 0;} while(0)
	#define	mSerDrv_On()	do{mSerDrv=0;} while(0)
	#define	mSerDrv_Off()	do{mSerDrv=1;} while(0)
	#define	mSerDrv_isOn()	(mSerDrv==0)

	/** USB FLAG *****************************************************/
#ifdef __18F25K50
	#define USBFLAG PIR3bits.USBIF
	#define USBPRIORITY IPR3bits.USBIP
	#define USB_INTERRUPTENABLE PIE3bits.USBIE
#else
	#define USBFLAG PIR2bits.USBIF
	#define USBPRIORITY IPR2bits.USBIP
	#define USB_INTERRUPTENABLE PIE2bits.USBIE
#endif

#endif
