/*********************************************************************
 *
 *                Analog library for Fraise pic18f device
 *				
 *				
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  april 2017     Original.
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
#include "softpwm.h"

#define SOFTPWM_MAX_CHANNELS 8

static unsigned int Value[SOFTPWM_MAX_CHANNELS];

// Timer macros
#define TIMER SOFTPWM_TIMER //5
#include <timer.h>

#define TIMER_INIT() do{\
	TIMER_CON = 0; \
	TIMER_PS0 = 1;  			/* 	prescaler 8 (->2MHz at 64MHz)	*/\
	TIMER_PS1 = 1;  			/* 	...								*/\
	TIMER_16BIT = 1;			/* 	16bits							*/\
	TIMER_IP = SOFTPWM_INTPRI;	/* 	high/low priority interrupt 	*/\
	TIMER_ON = 1;				/* 	stop timer						*/\
	TIMER_IF = 0;   			/*  clear flag						*/\
	TIMER_IE = 1;				/* 	enable timer interrupt			*/\
} while(0)

#define	TimerCountUS(T) (0xffff - ((T * FOSC) / 32000000)) //us ; 
#define InitTimerUS(T) do{TIMER_H = TimerCountUS(T) / 256; TIMER_L = TimerCountUS(T) % 256; TIMER_IF = 0;} while(0)

#define InitTimerUS(T) do{TIMER_H = (0xffff - T * 2) / 256; TIMER_L = (0xffff - T * 2) % 256; TIMER_IF = 0;} while(0)

void softpwmInit()
{
	unsigned char i;
		
	for(i = 0; i < SOFTPWM_MAX_CHANNELS; i++) {
		Value[i] = 0;
	}		
	TIMER_INIT();
#ifdef SOFTPWM0
	pinModeDigitalOut(SOFTPWM0);
#endif
#ifdef SOFTPWM1
	pinModeDigitalOut(SOFTPWM1);
#endif
#ifdef SOFTPWM2
	pinModeDigitalOut(SOFTPWM2);
#endif
#ifdef SOFTPWM3
	pinModeDigitalOut(SOFTPWM3);
#endif
#ifdef SOFTPWM4
	pinModeDigitalOut(SOFTPWM4);
#endif
#ifdef SOFTPWM5
	pinModeDigitalOut(SOFTPWM5);
#endif
#ifdef SOFTPWM6
	pinModeDigitalOut(SOFTPWM6);
#endif
#ifdef SOFTPWM7
	pinModeDigitalOut(SOFTPWM7);
#endif

}

void softpwmSet(unsigned char chan, unsigned int val)
{
	Value[chan] = val;
}

unsigned int softpwmGet(unsigned char chan)
{
	return Value[chan];
}

#define UPDATE_PIN(n,p) if(time < Value[n]) digitalSet(p); else digitalClear(p);

#if SOFTPWM_INTPRI == 1
void softpwmHighInterrupt(void)
#else
void softpwmLowInterrupt(void)
#endif
{
	static int time = SOFTPWM_MAX_VALUE;

	if(TIMER_IF) {
		if(time == 0) { 
			time = SOFTPWM_MAX_VALUE;
		}
		time--;
#ifdef SOFTPWM0
		UPDATE_PIN(0, SOFTPWM0);
#endif		
#ifdef SOFTPWM1
		UPDATE_PIN(1, SOFTPWM1);
#endif		
#ifdef SOFTPWM2
		UPDATE_PIN(2, SOFTPWM2);
#endif		
#ifdef SOFTPWM3
		UPDATE_PIN(3, SOFTPWM3);
#endif		
#ifdef SOFTPWM4
		UPDATE_PIN(4, SOFTPWM4);
#endif		
#ifdef SOFTPWM5
		UPDATE_PIN(5, SOFTPWM5);
#endif		
#ifdef SOFTPWM6
		UPDATE_PIN(6, SOFTPWM6);
#endif		
#ifdef SOFTPWM7
		UPDATE_PIN(7, SOFTPWM7);
#endif		
		InitTimerUS(SOFTPWM_TICK_US);
	}
}

#if SOFTPWM_INTPRI == 0
void softpwmHighInterrupt(void)
#else
void softpwmLowInterrupt(void)
#endif
{}

void softpwmReceive()
{
	unsigned char c, c2;
	unsigned int i = 0;
	
	c=fraiseGetChar();
	if(c < 8) {
		softpwmSet(c, fraiseGetInt());
	}
}

