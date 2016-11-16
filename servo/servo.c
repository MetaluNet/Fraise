/*********************************************************************
 *
 *                Servo library for Fraise pic18f  device
 *				Uses TIMER5 !
 *				
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
 * Antoine Rousseau  jan 2013     Original.
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

#include <servo.h>

static unsigned char *NextPort,*Port[8];
static unsigned char NextMask,Mask[8];
static unsigned int Val[8];
static unsigned char Count=0;

t_delay servoDelay;

#define SERVO_LOOP_TIME 18000 //micros

// Timer macros
#define TIMER 5

//determine PIE/PIR/IPR number
#if TIMER<=2
#define TIMERPNUM 1
#elif TIMER==3
#define TIMERPNUM 2
#else 
#define TIMERPNUM 5
#endif

#define TIMER_ON_(Timer) T##Timer##CONbits.TMR##Timer##ON
#define TIMER_ON CALL_FUN(TIMER_ON_,TIMER)

#define TIMER_IF_(Timer,TimerPNum) PIR##TimerPNum##bits.TMR##Timer##IF
#define TIMER_IF CALL_FUN2(TIMER_IF_,TIMER,TIMERPNUM)

#define TIMER_H_(Timer)  TMR##Timer##H
#define TIMER_H CALL_FUN(TIMER_H_,TIMER)

#define TIMER_L_(Timer)  TMR##Timer##L
#define TIMER_L CALL_FUN(TIMER_L_,TIMER)

#define TIMER_INIT_T(Timer,TimerPNum) \
	T##Timer##CON=0; \
	T##Timer##CONbits.T##Timer##CKPS0=1; 	/* 	prescaler 2 (->8MHz at 64MHz) 	*/\
	T##Timer##CONbits.T##Timer##RD16=1; 	/* 	16bits 				*/\
	PIE##TimerPNum##bits.TMR##Timer##IE=1;	/* 	enable timer interrupt 		*/\
	IPR##TimerPNum##bits.TMR##Timer##IP=1;	/* 	high priority 			*/\
	PIR##TimerPNum##bits.TMR##Timer##IF=0;


#define TIMER_INIT CALL_FUN2(TIMER_INIT_T,TIMER,TIMERPNUM)

void servoInit()
{
	unsigned char i;
	
	TIMER_INIT;
	
	for(i=0;i<8;i++) {
		Port[i]=NULL;
		Val[i]=0;
	}
	Count=0;
	
	delayStart(servoDelay, SERVO_LOOP_TIME);
}

void servoSetPort(unsigned char num,unsigned char *port,unsigned char mask)
{
	Port[num]=port;
	Mask[num]=mask;
	Val[num]=0;
}

void servoSet(unsigned char num,unsigned int val)
{
	Val[num]=val;
}

void servoRewind(void)
{
	Count=0;
}

void servoService(void)
{
	unsigned int val;
	
	if(delayFinished(servoDelay)) {
		servoRewind();
		delayStart(servoDelay, SERVO_LOOP_TIME);
	}	

	if(Count>7) return;
	if(TIMER_ON) return;
	
	if(Val[Count]==0) { 
		Count++;
		return;
	}
	
	NextPort=Port[Count];
	NextMask=Mask[Count];
	val=~Val[Count];
	TIMER_H=val>>8;
	TIMER_L=val&255;
	TIMER_IF=0;
	__critical {
		//*NextPort=(*NextPort)|NextMask;
		FSR0L=(unsigned char)NextPort;
		FSR0H=(unsigned int)NextPort>>8;
	
		INDF0|=NextMask;

		TIMER_ON=1;
	}
	Count++;	
}

void servoHighInterrupt(void)
{
	if(!TIMER_IF) return;
	/* *NextPort&=(~NextMask); */
	FSR0L=(unsigned char)NextPort;
	FSR0H=(unsigned int)NextPort>>8;
	
	INDF0&=(~NextMask);
	
	TIMER_ON=0;
	TIMER_IF=0;
}

void servoReceive()
{
	unsigned char c, c2;
	unsigned int i = 0;
	
	c=fraiseGetChar();
	if(c == 254) {
		fraiseSendCopy();
		c2=fraiseGetChar();
		if(c2 < 8) printf("%d %d\n",c2,Val[c2]);
	}
	else if(c < 8) {
		i = fraiseGetChar()<<8; 
		i += fraiseGetChar();
		Val[c]=i;
	}
}



