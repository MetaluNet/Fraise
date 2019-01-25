/*********************************************************************
 *
 *                Analog library for Fraise pic18f device
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
#include "analog.h"

#define CHANNELS_GROUPS ((ANALOG_MAX_CHANNELS+7)/8)

static int Value[ANALOG_MAX_CHANNELS];
static int oldValue[ANALOG_MAX_CHANNELS];
static int inValue[ANALOG_MAX_CHANNELS]; // input values
static int Max[ANALOG_MAX_CHANNELS];
static int Min[ANALOG_MAX_CHANNELS];
static int Dist[ANALOG_MAX_CHANNELS]; // distance of oldValue from inValue
static unsigned char Scaling=0; // 1 when scaling
static unsigned char Selected[CHANNELS_GROUPS];
static unsigned char HWChan[ANALOG_MAX_CHANNELS];
static unsigned char Pins[ANALOG_MAX_CHANNELS];
static int Threshold=ANALOG_THRESHOLD;
static char Mode = AMODE_NUM;

#define MINMAX_MARGIN ANALOG_MINMAX_MARGIN

/*#define bitset(var,bitno) ((var) |= (1 << (bitno)))
#define bitclr(var,bitno) ((var) &= ~(1 << (bitno)))
#define bittst(var,bitno) (var & (1 << (bitno)))*/

#define isSelected(num) (bittst(Selected[num>>3],num&7))

void analogInit()
{
	unsigned char i;
		
	for(i=0;i<CHANNELS_GROUPS;i++) {
		Selected[i]=0;
	}
	
	for(i=0;i<ANALOG_MAX_CHANNELS;i++) {
		Value[i]=0;
		inValue[i]=0;
		oldValue[i]=0;
		Min[i]=0;
		Max[i]=0x7FFF;
		Pins[i]=255;
	}
	
	ADCON2=0b10101111; //right justified, 12 AD, RC
	ADCON1=0; //0->Vdd
	ADCON0=1; //on, channel 0
	
	ADCON0bits.GO=1; // go !

}

void analogInitTouch(){
	CTMUCONH = 0x00;
	CTMUCONL = 0x90;
	CTMUICON = 0x03;			//55uA
	CTMUCONHbits.CTMUEN = 1;	//Enable CTMU
}

void analogSelectAdc(unsigned char chan,unsigned char hwchan )
{
	bitset(Selected[chan>>3],chan&7);
	HWChan[chan]=hwchan;
	Pins[chan]=255;
}

void analogSelectAdcTouch(unsigned char chan,unsigned char hwchan, unsigned char *port, unsigned char bit)
{
	bitset(Selected[chan>>3],chan&7);
	HWChan[chan]=hwchan;
	Pins[chan]=(((unsigned int)(port-&PORTA)&7)<<4) + (bit&7);
}

void analogDeselect(unsigned char chan)
{
	bitclr(Selected[chan>>3],chan&7);
}

#ifdef ANALOG_CHANNEL_PRECONF
extern void ANALOG_CHANNEL_PRECONF(unsigned char chan);
#endif

unsigned char analogService(void)
{
	static unsigned char chan=0, conv=0;
	int v;
	unsigned char pin;
	
	if(ADCON0bits.GO) return chan;
	
	if(conv) {
		v = Value[chan];
// v = oldv-oldv/N+ADres = ((ADres*N)+oldv*(N-1))/N : v = N*lowpass[N](ADres) N=2^ANALOG_FILTER
		pin = Pins[chan];
		if(pin != 255) {
			bitclr(*(&TRISA+(pin>>4)),pin&7);
			bitclr(*(&LATA+(pin>>4)),pin&7);
		}
		Value[chan] = v - (v>>ANALOG_FILTER) + ADRESL+(ADRESH<<8); 
		if(Scaling == 1) {
			v = Value[chan];
			if(v < (Min[chan]-MINMAX_MARGIN)) Min[chan] = v+MINMAX_MARGIN;
			if(v > (Max[chan]+MINMAX_MARGIN)) Max[chan] = v-MINMAX_MARGIN;
		}
		conv=0;
	}
	
	chan++;
	if(chan >= ANALOG_MAX_CHANNELS) chan = 0;
	
	if(isSelected(chan))	{
		ADCON0=(HWChan[chan] << 2) + 1;
#ifdef ANALOG_CHANNEL_PRECONF
		ANALOG_CHANNEL_PRECONF(chan);
#endif
		pin = Pins[chan];
		if(pin != 255) {
			bitset(*(&TRISA+(pin>>4)),pin&7);// set channel to digital input
			bitset(*(__data unsigned char*)((int)&ANSELA + (pin>>4)),pin&7);// set channel to analog input
			CTMUCONHbits.IDISSEN = 1;		// Drain any charge on the A/D circuit
			Nop(); Nop(); 
			CTMUCONHbits.IDISSEN =  0;		// Stop discharge of A/D circuit
			CTMUCONLbits.EDG2STAT = 0;		// Make sure edge2 is 0
			__critical{
				CTMUCONLbits.EDG1STAT = 1;	// Set edge1 - Start Charge
				Nop(); Nop(); Nop(); Nop();
				Nop(); Nop(); Nop(); Nop();
				Nop(); Nop(); Nop(); Nop();
				Nop(); Nop(); Nop(); Nop();
				Nop(); Nop(); Nop(); Nop();	
				CTMUCONLbits.EDG1STAT = 0;	//Clear edge1 - Stop Charge
			}			
		}
		ADCON0bits.GO = 1;
		conv = 1;
	}
	
	return chan;
}

char analogSend()
{
	static unsigned char chan = 0;
	unsigned char loop = 0, count = 0, len = 0;
	int v, d;
	static unsigned char buf[13] = { 'B' }; //'B' + 3x4 chars
	
	while((count < 4) && (loop++ < ANALOG_MAX_CHANNELS)){
		chan++;
		if(chan >= ANALOG_MAX_CHANNELS) {
			chan = 0;
		}
		if(! isSelected(chan)) continue;

		v = Value[chan];
		if(Mode & AMODE_SCALE) { // scale :
			if(v < Min[chan]) v = 0;
			else if(v > Max[chan]) v = ANALOG_SCALED_MAX;
			else v = (unsigned int)( 
				((ANALOG_SCALED_MAX+1UL) * (v - Min[chan])) / 
				(Max[chan] - Min[chan]) );

			if(v <= Threshold ) v = 0;
			else if(v >= (ANALOG_SCALED_MAX - Threshold)) 
				v = ANALOG_SCALED_MAX;
		} 
		d = v - oldValue[chan];
		if(d < 0) d = -d;
		
		count++;
		
		if(d <= Threshold) continue;

		oldValue[chan] = v;
		
		if(Mode & AMODE_CROSS) {
			if(Dist[chan] != 0) {
				d = v - inValue[chan];
				if((d != 0) && ((d > 0) == (Dist[chan] > 0))) { 
					Dist[chan] = d;
					continue; 
				}
				else Dist[chan] = 0;
			}
		}
		
		if(Mode & AMODE_NUM) {
			if( !len ) buf[len++] = 'B';
			buf[len++] = chan + 100;
			buf[len++] = v >> 8;
			buf[len++] = v & 255;
		}
		else printf("C A %d %d\n",chan,v);
	}
	
	if((len != 0) && (Mode & AMODE_NUM)) {
		buf[len++] = '\n';
		fraiseSend(buf,len);
	}
	
	return count;		
}

void analogSetMode(unsigned char mode) // scale : bit0 = scale_on ; bit1 = num_on(=text_off) ; bit2 = cross_inval
{
	Mode = mode;
}

void analogSet(unsigned char chan, int val)
{
	int d;
	
	inValue[chan] = val;
	d = oldValue[chan] - val;
	Dist[chan] = d;
	if(d < 0) d = -d;
	if(d < 3) d = 0;
	if(d == 0) Dist[chan] = 0;
}

int analogGet(unsigned char chan)
{
	return Value[chan];
}
	
int analogGetDistance(unsigned char chan)
{
	return Dist[chan];
}

void analogScaling(unsigned char scaling) // when scaling, min and max are updated each sample
{
	unsigned char i;
	
	if(scaling==1) {
		for(i=0;i<ANALOG_MAX_CHANNELS;i++) {
			Min[i]=0x7FFF;
			Max[i]=0;
		}
	}
	
	Scaling=scaling;
}

#define EEDECLARE(x) \
	EEdeclareInt(&Max[x]); \
	EEdeclareInt(&Min[x])

void analogDeclareEE()
{
#if ANALOG_MAX_CHANNELS > 0
	EEDECLARE(0);
	EEDECLARE(1);
	EEDECLARE(2);
	EEDECLARE(3);
#endif
#if ANALOG_MAX_CHANNELS > 4
	EEDECLARE(4);
	EEDECLARE(5);
	EEDECLARE(6);
	EEDECLARE(7);
#endif
#if ANALOG_MAX_CHANNELS > 8
	EEDECLARE(8);
	EEDECLARE(9);
	EEDECLARE(10);
	EEDECLARE(11);
#endif
#if ANALOG_MAX_CHANNELS > 12
	EEDECLARE(12);
	EEDECLARE(13);
	EEDECLARE(14);
	EEDECLARE(15);
#endif
	
}

