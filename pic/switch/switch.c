/*********************************************************************
 *
 *                Switch library for Fraise pic18f device
 *				
 *				
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  march 2013     Original.
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

#include <config.h>
#include <switch.h>

#define MAX_CHANNELS 16
#define CHANNELS_GROUPS ((MAX_CHANNELS+7)/8)

static unsigned char Pins[MAX_CHANNELS];

static unsigned char States[CHANNELS_GROUPS];
static unsigned char oldStates[CHANNELS_GROUPS];

#define CHANISSET(num) (bittst(States[num>>3],num&7))
#define CHANCLR(num)  (bitclr(States[num>>3],num&7))
#define CHANSET(num) (bitset(States[num>>3],num&7))

#define OLDCHANISSET(num) (bittst(oldStates[num>>3],num&7))
#define OLDCHANCLR(num)  (bitclr(oldStates[num>>3],num&7))
#define OLDCHANSET(num) (bitset(oldStates[num>>3],num&7))

void switchInit()
{
	unsigned char i;
		
	for(i=0;i<CHANNELS_GROUPS;i++) {
		oldStates[i] = States[i] = 1;
	}
	
	for(i=0;i<MAX_CHANNELS;i++) {
		Pins[i] = 255;
	}
}

void switchSelectHW(unsigned char channel, unsigned char *port, unsigned char bit)
{
	Pins[channel] = (((unsigned int)(port-&PORTA)&7)<<4) + (bit&7);
}

void switchDeselect(unsigned char channel)
{
	Pins[channel] = 255;
}

void switchService(void)
{
	unsigned char i,pin;
	
	for(i=0;i<MAX_CHANNELS;i++) {
		pin=Pins[i];
		if(pin != 255) {
			if(bittst(*(&PORTA+(pin>>4)),pin&7)) CHANSET(i);
			else CHANCLR(i);
		}
	}	
}

char switchSend(void)
{
	static unsigned char chan=0;
	unsigned char count=0,pin,set;
	
	while(count<4) {
		pin=Pins[chan];
		if(pin != 255) {
			set=CHANISSET(chan);
			if(set!=OLDCHANISSET(chan)) {
				if(set) OLDCHANSET(chan);
				else OLDCHANCLR(chan);
				printf("C s %d %d\n",chan,set==0);
				count++;
			}
		}
		chan++;
		if(chan>=MAX_CHANNELS) {
			chan=0;
			break;
		}
	}
	return count;
}

char switchGet(unsigned char chan)
{
	return CHANISSET(chan);
}


