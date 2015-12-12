/*********************************************************************
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

 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) Antoine Rousseau   nov 2011 
 ********************************************************************/

#ifndef CORE_H
#define CORE_H

/** E X T E R N S ************************************************************/

/** P U B L I C  P R O T O T Y P E S *****************************************/
#include <config.h>
#include <pic18fregs.h>
#include <boardconfig.h>
#include <boardio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <typedefs.h>

void coreInit();
//User defined main interrupt routines :
void highInterrupts();
void lowInterrupts();

void eeWriteByte(unsigned char address, unsigned char value);
unsigned char eeReadByte(unsigned char address);


#define CALL_FUN(x,a) x(a)
#define CALL_FUN2(x,a,b) x(a,b)
#define CALL_FUN3(x,a,b,c) x(a,b,c)
#define CALL_FUN4(x,a,c,d) x(a,b,c,d)
//#define CALL_FUNX(x,args...) x(args)

// connector macros :
 
#define KPORT(k) k##PORT
#define KBIT(k)  k##BIT
#define KAN(k)  k##AN

//----- Digital Read :
#define DIGITALREAD_(connport,connbit) (PORT##connport##bits.R##connport##connbit)
#define digitalRead(conn) CALL_FUN2(DIGITALREAD_,KPORT(conn),KBIT(conn))

//----- Digital Write :
#define DIGITALWRITE_(connport,connbit,val) do{ \
	if((val) != 0) LAT##connport##bits.LAT##connport##connbit = 1; \
	else LAT##connport##bits.LAT##connport##connbit = 0; \
}while(0)
#define digitalWrite(conn,val) CALL_FUN3(DIGITALWRITE_,KPORT(conn),KBIT(conn),val)

#define DIGITALCLEAR_(connport,connbit) do{ LAT##connport##bits.LAT##connport##connbit = 0; }while(0)
#define digitalClear(conn) CALL_FUN2(DIGITALCLEAR_,KPORT(conn),KBIT(conn))

#define DIGITALSET_(connport,connbit) do{ LAT##connport##bits.LAT##connport##connbit = 1; }while(0)
#define digitalSet(conn) CALL_FUN2(DIGITALSET_,KPORT(conn),KBIT(conn))

//----- Pin Mode :
#define PIN_MODE_DIGITAL_IN_(connport,connbit) do{\
	TRIS##connport##bits.TRIS##connport##connbit=1;\
	bitclr(*(__data unsigned char*)((int)&PORT##connport + (int)&ANSELA-(int)&PORTA),connbit);\
 } while(0)
#define pinModeDigitalIn(conn) CALL_FUN2(PIN_MODE_DIGITAL_IN_,KPORT(conn),KBIT(conn))

#define PIN_MODE_ANALOGIN_(connport,connbit) do{\
	TRIS##connport##bits.TRIS##connport##connbit=1;\
	bitset(*(__data unsigned char*)((int)&PORT##connport + (int)&ANSELA-(int)&PORTA),connbit);\
 } while(0)
#define pinModeAnalogIn(conn) CALL_FUN2(PIN_MODE_ANALOGIN_,KPORT(conn),KBIT(conn))

#define PIN_MODE_DIGITAL_OUT_(connport,connbit) do{\
	TRIS##connport##bits.TRIS##connport##connbit=0;\
 } while(0)
#define pinModeDigitalOut(conn) CALL_FUN2(PIN_MODE_DIGITAL_OUT_,KPORT(conn),KBIT(conn))

/*#define SET_PIN_ANSEL_(connport,connbit,val) do{ \
		ANSEL##connport##bits.ANS##connport##connbit=((val)!=0);\
	} while(0)
		
#define setPinAnsel(conn,val) CALL_FUN3(SET_PIN_ANSEL_,KPORT(conn),KBIT(conn),(val)!=0)*/

// bit macros: 

#define BIT_COPY(dest,src) do{ if((src)!=0) dest = 1 ; else dest = 0 ; } while(0)

#define bitset(var,bitno) ((var) |= (1 << (bitno)))
#define bitclr(var,bitno) ((var) &= ~(1 << (bitno)))
#define bittst(var,bitno) (unsigned char)((var & (1 << (bitno)))!=0)


//------------------------- Time : ----------------
unsigned unsigned long int time(void); // in 64/FOSC steps (1us @ 64MHz, 8us @ 8MHz)
extern volatile DWORD Now; 	       // time at last high interrupt
#define timeISR() (Now._dword)
#define elapsed(since) ((time()-(unsigned long)(since))&0x7FFFFFFF) // in time cycles
#define	microToTime(T) (((unsigned long)T*(FOSC/64000UL))/1000) //microseconds to time cycles
#define	timeToMicro(T) (((unsigned long)T*(FOSC/64000UL))/1000) //time cycles to microseconds


#ifndef abs
#define abs(x) ((x)<0?-(x):(x))
#endif

typedef  unsigned long t_delay;
#define delayStart(delay, micros) delay = time() +  microToTime(micros)
#define delayFinished(delay) (elapsed(delay) < 0x3FFFFFFF)

#endif //FRAISE_H
