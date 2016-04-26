/*********************************************************************
 *
 *                Dimmer library for Fraise pic18f  device
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

#include <dimmer.h>

unsigned int Val[8];            // channels values

// following values are duplicated for page swaping
unsigned char first[2];        // first channel to go (128 if none)
unsigned int firstTime[2];      // time of the first channel  
unsigned char follower[16];     // channel following each channel (8 when last)
unsigned int followerTime[16];  // values of the sorted ports

union{
	unsigned char flags;
    struct {
        unsigned page:1;    // page number to be used by interrupt
        unsigned pageInt:1; // page currently used by interrupt
        unsigned changed:1; // one (or more) channel value changed
    };
} status;

// Timer macros
#define TIMER DIMMER_TIMER //5
#include <timer.h>

#define TIMER_INIT() do{\
	TIMER_CON = 0; \
	TIMER_PS1 = 1;  /* 	prescaler 4 (->4MHz at 64MHz)   */\
	TIMER_16BIT = 1;/* 	16bits                          */\
	TIMER_IP = 1;	/* 	high priority                   */\
	TIMER_ON = 0;	/* 	stop timer                      */\
	TIMER_IF = 0;   /*  clear flag                      */\
	TIMER_IE = 1;	/* 	enable timer interrupt         */\
} while(0)

// Timer macros
#define INTPIN KINT(DIMMER_INTPIN)
#include <intpin.h>

void dimmerInit()
{
	unsigned char i;
	
	status.flags = 0;
	TIMER_INIT();

	for(i = 0; i < 8; i++) {
		Val[i] = 65535;
	}

	pinModeDigitalIn(DIMMER_INTPIN);
	INTPIN_EDGE = DIMMER_INTEDGE;
	INTPIN_IP = 1; // high priority
	INTPIN_IF = 0; // clear flag
	INTPIN_IE = 0; // enable interrupt

    digitalClear(DIMMER_K0);
    digitalClear(DIMMER_K1);
    digitalClear(DIMMER_K2);
    digitalClear(DIMMER_K3);
    digitalClear(DIMMER_K4);
    digitalClear(DIMMER_K5);
    digitalClear(DIMMER_K6);
    digitalClear(DIMMER_K7);
    pinModeDigitalOut(DIMMER_K0);
    pinModeDigitalOut(DIMMER_K1);
    pinModeDigitalOut(DIMMER_K2);
    pinModeDigitalOut(DIMMER_K3);
    pinModeDigitalOut(DIMMER_K4);
    pinModeDigitalOut(DIMMER_K5);
    pinModeDigitalOut(DIMMER_K6);
    pinModeDigitalOut(DIMMER_K7);
}

void dimmerSet(unsigned char num,unsigned int val)
{
	Val[num] = val;
	status.changed = 1;
}

void dimmerService(void)
{
    unsigned char i, j, tmp;
    static unsigned char sorted[8];
    
	if( (!status.changed) || (status.page != status.pageInt)) return;	

    status.changed = 0;
    
    for(i = 0 ; i < 8 ; i++) {
        sorted[i] = i;
    }
    
    // sort channels ascendingly, by insertion algorithm (https://en.wikipedia.org/wiki/Insertion_sort)
    for(i = 1 ; i < 8 ; i++) {
        j = i;
        while((j > 0) && (Val[sorted[j-1]] > Val[sorted[j]])) {
            tmp = sorted[j-1];
            sorted[j-1] = sorted[j];
            sorted[j] = tmp;
            j--;
        }
    }
    
    first[!status.page] = sorted[0];
    firstTime[!status.page] = Val[sorted[0]];

    for(i = 0 ; i < 7 ; i++) {
        follower[sorted[i]] = sorted[i+1];
        followerTime[sorted[i]] = Val[sorted[i+1]] - Val[sorted[i]];
    }
    follower[sorted[7]] = 8;
    
    status.page = !status.page; 
}

#define PROCESS_CHAN(chan, page) case chan : \
    digitalSet(DIMMER_K##chan); \
    next = follower[chan + page?8:0] ; \
    if(!(next & 8)) { \
	    TIMER_H = (followerTime[chan + page?8:0]) >> 8; \
	    TIMER_L = (followerTime[chan + page?8:0]) & 255; \
        TIMER_IE = 1; \
        TIMER_ON = 1; \
    } \
    break

void dimmerHighInterrupt(void)
{
	static unsigned char next;
	static unsigned int val;
	
	if(INTPIN_IF){
	    INTPIN_IF = 0;
	    status.pageInt = status.page;
	    digitalClear(DIMMER_K0);
	    digitalClear(DIMMER_K1);
	    digitalClear(DIMMER_K2);
	    digitalClear(DIMMER_K3);
	    digitalClear(DIMMER_K4);
	    digitalClear(DIMMER_K5);
	    digitalClear(DIMMER_K6);
	    digitalClear(DIMMER_K7);
	    
	    next = status.pageInt ? first[1] : first[0];
        val = status.pageInt ? firstTime[1] : firstTime[0];
        TIMER_ON = 0;
        TIMER_H = val>>8;
        TIMER_L = val&255;
        TIMER_IF = 0;
        TIMER_IE = 1;
        TIMER_ON = 1;
	}
	
	if(TIMER_IF) {
	    TIMER_ON = 0;
	    TIMER_IF = 0;
	    if(status.pageInt) switch(next) {
	        PROCESS_CHAN(0, 1);
	        PROCESS_CHAN(1, 1);
	        PROCESS_CHAN(2, 1);
	        PROCESS_CHAN(3, 1);
	        PROCESS_CHAN(4, 1);
	        PROCESS_CHAN(5, 1);
	        PROCESS_CHAN(6, 1);
	        PROCESS_CHAN(7, 1);
	    } else switch(next) {
	        PROCESS_CHAN(0, 0);
	        PROCESS_CHAN(1, 0);
	        PROCESS_CHAN(2, 0);
	        PROCESS_CHAN(3, 0);
	        PROCESS_CHAN(4, 0);
	        PROCESS_CHAN(5, 0);
	        PROCESS_CHAN(6, 0);
	        PROCESS_CHAN(7, 0);
	    }
	}
}

void dimmerReceive()
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



