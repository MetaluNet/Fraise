/* ********************************************************************
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

*********************************************************************
 * Copyright (c) Antoine Rousseau   nov 2011 
 ******************************************************************* */

#ifndef CORE_H
#define CORE_H

/* E X T E R N S *********************************************************** */

/* P U B L I C  P R O T O T Y P E S **************************************** */
#include <config.h>
#include <pic18fregs.h>
#include <boardconfig.h>
#include <boardio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <typedefs.h>

/** @defgroup core Core module
 *  Core module implements eeprom, pins and time.
 *  @{
 */

/** @name Initialization
 @{ */
/** @brief Init core module. */
/** Init processor, configure TIMER0 for time() use.
* Normally coreInit() is called by fruitInit() (see fruit module), so you don't have to call it by yourself.
*/
void coreInit();
/**@}*/

/**@name User defined functions 
@{*/
/** @brief User defined initialization. */
void setup();
/** @brief User defined forever loop. */
void loop();

/** @brief Optional user defined high priority interrupt routine. */
void highInterrupts();
/** @brief Optional user defined low priority interrupt routine. */
void lowInterrupts();
/**@}*/

/* @brief Write to eeprom. */
void eeWriteByte(unsigned char address, unsigned char value);
/* @brief Write to eeprom. */
unsigned char eeReadByte(unsigned char address);


#define CALL_FUN(x,a) x(a)
#define CALL_FUN2(x,a,b) x(a,b)
#define CALL_FUN3(x,a,b,c) x(a,b,c)
#define CALL_FUN4(x,a,b,c,d) x(a,b,c,d)
//#define CALL_FUNX(x,args...) x(args)

// connector macros :
 
#define KPROP(k, prop) k##prop
#define KPORT(k) CALL_FUN2(KPROP, k, PORT)
#define KBIT(k)  CALL_FUN2(KPROP, k, BIT)
#define KAN(k)  CALL_FUN2(KPROP, k, AN)
#define KINT(k)  CALL_FUN2(KPROP, k, INT)
#define KPWM(k)  CALL_FUN2(KPROP, k, PWM)
#define KSETUP_PWM(k)  CALL_FUN2(KPROP, k, SETUP_PWM)

// PIN ROUTINES 
//----- Digital Read :
#define DIGITALREAD_(connport,connbit) (PORT##connport##bits.R##connport##connbit)
//----- Digital Write :
#define DIGITALWRITE_(connport,connbit,val) do{ \
	if((val) != 0) LAT##connport##bits.LAT##connport##connbit = 1; \
	else LAT##connport##bits.LAT##connport##connbit = 0; \
}while(0)
#define DIGITALCLEAR_(connport,connbit) do{ LAT##connport##bits.LAT##connport##connbit = 0; }while(0)
#define DIGITALSET_(connport,connbit) do{ LAT##connport##bits.LAT##connport##connbit = 1; }while(0)
//----- Pin Mode :
#define PIN_MODE_DIGITAL_IN_(connport,connbit) do{\
	TRIS##connport##bits.TRIS##connport##connbit=1;\
	bitclr(*(__data unsigned char*)((int)&PORT##connport + (int)&ANSELA-(int)&PORTA),connbit);\
 } while(0)
#define PIN_MODE_ANALOGIN_(connport,connbit) do{\
	TRIS##connport##bits.TRIS##connport##connbit=1;\
	bitset(*(__data unsigned char*)((int)&PORT##connport + (int)&ANSELA-(int)&PORTA),connbit);\
 } while(0)
#define PIN_MODE_DIGITAL_OUT_(connport,connbit) do{\
	TRIS##connport##bits.TRIS##connport##connbit=0;\
 } while(0)
//----- PWM ("analog" output):
#define PIN_MODE_ANALOG_OUT_(connport,connbit,pwm,setup_pwm) do{\
	TRIS##connport##bits.TRIS##connport##connbit=0;\
	setup_pwm(); /* pin special PWM setup, e.g select right steering output if needed */\
	CCP##pwm##CON = 0b00001100; /* single PWM active high*/\
 } while(0)

#define ANALOG_WRITE_(pwm,val) do{ CCP##pwm##CONbits.DC##pwm##B1 = val&2; CCP##pwm##CONbits.DC##pwm##B0 = val&1; CCPR##pwm##L=val>>2; } while(0)



/** \name Pin routines */
//@{

/** @brief Set pin mode to Digital Input.
    @param conn Symbol of the pin (example : K1 for connector 1)	
*/
#define pinModeDigitalIn(conn) CALL_FUN2(PIN_MODE_DIGITAL_IN_,KPORT(conn),KBIT(conn))

/** @brief Set pin mode to Analog Input.
    @param conn Symbol of the pin (example: K1 for connector 1)	
*/
#define pinModeAnalogIn(conn) CALL_FUN2(PIN_MODE_ANALOGIN_,KPORT(conn),KBIT(conn))

/** @brief Set pin mode to Digital Output. 
    @param conn Symbol of the pin (example: K1 for connector 1)	
*/
#define pinModeDigitalOut(conn) CALL_FUN2(PIN_MODE_DIGITAL_OUT_,KPORT(conn),KBIT(conn))

/** @brief Set pin mode to Analog Output (PWM). 
    @param conn Symbol of the pin (example: K1 for connector 1)	
*/
#define pinModeAnalogOut(conn) CALL_FUN4(PIN_MODE_ANALOG_OUT_, KPORT(conn), KBIT(conn), KPWM(conn), KSETUP_PWM(conn))

/** @brief Digital read from pin. 
	Get the voltage (0/1) seen from the pin.
    @param conn Symbol of the pin (example: K1 for connector 1)
    @return 0 if voltage was LOW	
    @return 1 if voltage was HIGH	
*/
#define digitalRead(conn) CALL_FUN2(DIGITALREAD_,KPORT(conn),KBIT(conn))

/** @brief Digital write to pin. 
	Set the pin output voltage to LOW or HIGH.
    @param conn Symbol of the pin (example: K1 for connector 1)	
    @param val 0 or 1
*/
#define digitalWrite(conn,val) CALL_FUN3(DIGITALWRITE_,KPORT(conn),KBIT(conn),val)

/** @brief Digital clear pin. 
	Clear the pin output voltage.
    @param conn Symbol of the pin (example: K1 for connector 1)	
*/
#define digitalClear(conn) CALL_FUN2(DIGITALCLEAR_,KPORT(conn),KBIT(conn))

/** @brief Digital set pin. 
	Set the pin output voltage ot HIGH.
    @param conn Symbol of the pin (example: K1 for connector 1)	
*/
#define digitalSet(conn) CALL_FUN2(DIGITALSET_,KPORT(conn),KBIT(conn))

/** @brief 'Analog' write to pin. 
	Set the pin output voltage (actually PWM ratio).
    @param conn Symbol of the pin (example: K1 for connector 1)	
    @param val Value between 0 and 1023
*/
#define analogWrite(conn,val) CALL_FUN2(ANALOG_WRITE_, KPWM(conn), val)

//@}

/** \name Bit macro */
//@{
#define BIT_COPY(dest,src) do{ if((src)!=0) dest = 1 ; else dest = 0 ; } while(0) ///< Copy bit **src** to **dest**.

#define bitset(var,bitno) ((var) |= (1 << (bitno))) ///< Set bit **bitno** in variable **var**.
#define bitclr(var,bitno) ((var) &= ~(1 << (bitno))) ///< Clear bit **bitno** in variable **var**.
#define bittst(var,bitno) (unsigned char)((var & (1 << (bitno)))!=0) ///< Get value of bit **bitno** in variable **var**.
//@}

#ifndef abs
#define abs(x) ((x)<0?-(x):(x))
#endif

//------------------------- Time : ----------------
extern volatile DWORD Now; 	       // time at last high interrupt
#define elapsed(since) ((time()-(unsigned long)(since))&0x7FFFFFFF) // in time cycles
#define elapsedISR(since) ((timeISR()-(unsigned long)(since))&0x7FFFFFFF) // in time cycles
#define	microToTime(T) (((unsigned long)T*(FOSC/64000UL))/1000) //microseconds to time cycles
#define	timeToMicro(T) (((unsigned long)T*(FOSC/64000UL))/1000) //time cycles to microseconds

/** \name Time functions */
//@{
unsigned unsigned long int time(void); ///< Get time since bootup in 64/FOSC steps (1us @ 64MHz, 8us @ 8MHz).

#define timeISR() (Now._dword) ///< time() equivalent to be used inside of interrupts routines.

/// Time type
typedef  unsigned long t_time;

/// Delay type
typedef  unsigned long t_delay;

/// @brief Start delay
/// @param delay Delay to start (use t_delay type)
/// @param micros Timeout in microseconds
#define delayStart(delay, micros) delay = time() +  microToTime(micros)

/// @brief Test delay timeout 
/// @param delay Delay to test (use t_delay type)
/// @return TRUE if timeout
#define delayFinished(delay) (elapsed(delay) < 0x3FFFFFFF)

//@}

//----------------------- fake port Z ---------------
typedef union
  {
  struct
    {
    unsigned RZ0                : 1;
    unsigned RZ1                : 1;
    unsigned RZ2                : 1;
    unsigned RZ3                : 1;
    unsigned RZ4                : 1;
    unsigned RZ5                : 1;
    unsigned RZ6                : 1;
    unsigned RZ7                : 1;
    };
  } __PORTZbits_t;

typedef union
  {
  struct
    {
    unsigned LATZ0              : 1;
    unsigned LATZ1              : 1;
    unsigned LATZ2              : 1;
    unsigned LATZ3              : 1;
    unsigned LATZ4              : 1;
    unsigned LATZ5              : 1;
    unsigned LATZ6              : 1;
    unsigned LATZ7              : 1;
    };
  } __LATZbits_t;

typedef union
  {
  struct
    {
    unsigned TRISZ0             : 1;
    unsigned TRISZ1             : 1;
    unsigned TRISZ2             : 1;
    unsigned TRISZ3             : 1;
    unsigned TRISZ4             : 1;
    unsigned TRISZ5             : 1;
    unsigned TRISZ6             : 1;
    unsigned TRISZ7             : 1;
    };

  struct
    {
    unsigned RZ0                : 1;
    unsigned RZ1                : 1;
    unsigned RZ2                : 1;
    unsigned RZ3                : 1;
    unsigned RZ4                : 1;
    unsigned RZ5                : 1;
    unsigned RZ6                : 1;
    unsigned RZ7                : 1;
    };
  } __TRISZbits_t;


typedef struct
  {
  unsigned ANSZ0                : 1;
  unsigned ANSZ1                : 1;
  unsigned ANSZ2                : 1;
  unsigned ANSZ3                : 1;
  unsigned ANSZ4                : 1;
  unsigned ANSZ5                : 1;
  unsigned ANSZ6                : 1;
  unsigned ANSZ7                : 1;
  } __ANSELZbits_t;

#define _PORTZ_ADDR 0x0480
#define _PORT_TO_LAT 0x09
#define _PORT_TO_TRIS 0x12
#define _PORT_TO_ANSEL (-0x48)

extern __at(_PORTZ_ADDR) volatile unsigned char PORTZ;
extern __at(_PORTZ_ADDR) volatile __PORTZbits_t PORTZbits;

extern __at(_PORTZ_ADDR + _PORT_TO_LAT) volatile unsigned char LATZ;
extern __at(_PORTZ_ADDR + _PORT_TO_LAT) volatile __LATZbits_t LATZbits;

extern __at(_PORTZ_ADDR + _PORT_TO_TRIS) volatile unsigned char TRICZ;
extern __at(_PORTZ_ADDR + _PORT_TO_TRIS) volatile __TRISZbits_t TRISZbits;

extern __at(_PORTZ_ADDR + _PORT_TO_ANSEL) volatile unsigned char ANSELZ;
extern __at(_PORTZ_ADDR + _PORT_TO_ANSEL) volatile __ANSELZbits_t ANSELZbits;

//----------------------- fake port Z connectors ---------------

#define KZ0PORT Z	// PORTZ0 & LATZ0 are initialized to 0 
#define KZ0BIT 	0

#define KZ1PORT Z	// PORTZ1 & LATZ1 are initialized to 1 
#define KZ1BIT 	1

#define KZ2PORT Z
#define KZ2BIT 	2

#define KZ3PORT Z
#define KZ3BIT 	3

#define KZ4PORT Z
#define KZ4BIT 	4

#define KZ5PORT Z
#define KZ5BIT 	5

#define KZ6PORT Z
#define KZ6BIT 	6

#define KZ7PORT Z
#define KZ7BIT 	7

/** @} 
*/

#endif //FRAISE_H
