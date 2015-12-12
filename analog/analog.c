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

#define MAX_CHANNELS 8
#define CHANNELS_GROUPS ((MAX_CHANNELS+7)/8)

static int Value[MAX_CHANNELS];
static int oldValue[MAX_CHANNELS];
static int inValue[MAX_CHANNELS]; // input values
static int Max[MAX_CHANNELS];
static int Min[MAX_CHANNELS];
static int Dist[MAX_CHANNELS]; // distance of oldValue from inValue
static unsigned char Scaling=0; // 1 when scaling
static unsigned char Selected[CHANNELS_GROUPS];
static unsigned char HWChan[MAX_CHANNELS];
static int Threshold=ANALOG_THRESHOLD;

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
	
	for(i=0;i<MAX_CHANNELS;i++) {
		Value[i]=0;
		inValue[i]=0;
		oldValue[i]=0;
		Min[i]=0;
		Max[i]=0x7FFF;
	}
	
	ADCON2=0b10101111; //right justified, 12 AD, RC
	ADCON1=0; //0->Vdd
	ADCON0=1; //on, channel 0
	
	ADCON0bits.GO=1; // go !

}

void analogSelectAdc(unsigned char chan,unsigned char hwchan )
{
	bitset(Selected[chan>>3],chan&7);
	HWChan[chan]=hwchan;
}

void analogDeselect(unsigned char chan)
{
	bitclr(Selected[chan>>3],chan&7);
}

void analogService(void)
{
	static unsigned char chan=0, conv=0;
	int v;
	
	if(ADCON0bits.GO) return;
	
	if(conv) {
		v = Value[chan];
		//Value[chan]= v - (v>>3) + ADRESL+(ADRESH<<8); // v = oldv-oldv/8+adres = ((adres*8)+oldv*7)/8 : v = lowpass(adres*8)
		Value[chan] = v - (v>>ANALOG_FILTER) + ADRESL+(ADRESH<<8); // v = oldv-oldv/N+adres = ((adres*N)+oldv*(N-1))/N : v = lowpass[N](adres*N) N=2^ANALOG_FILTER
		if(Scaling == 1) {
			v = Value[chan];
			if(v < (Min[chan]-MINMAX_MARGIN)) Min[chan] = v+MINMAX_MARGIN;
			if(v > (Max[chan]+MINMAX_MARGIN)) Max[chan] = v-MINMAX_MARGIN;
		}
		conv=0;
	}
	
	chan++;
	if(chan >= MAX_CHANNELS) chan = 0;
	
	if(isSelected(chan))	{
		ADCON0=(HWChan[chan] << 2) + 1;
		ADCON0bits.GO = 1;
		conv = 1;
	}
}

char analogSend(unsigned char mode) // scale : bit0 = scale_on ; bit1 = num_on(=text_off) ; bit2 = cross_inval
{
	static unsigned char chan = 0;
	unsigned char count = 0, len = 0;
	int v, d;
	static unsigned char buf[13] = { 'B' }; //'B' + 3x4 chars
	
	while(count < 4) {
		chan++;
		if(chan >= MAX_CHANNELS) {
			chan = 0;
		}
		if(! isSelected(chan)) continue;

		v = Value[chan];
		if(mode & AMODE_SCALE) { // scale :
			if(v < Min[chan]) v = 0;
			else if(v > Max[chan]) v = ANALOG_SCALED_MAX;
			else v = (unsigned int)( ( (ANALOG_SCALED_MAX+1UL) * (v - Min[chan])) / (Max[chan] - Min[chan]) );

			if(v <= Threshold ) v = 0;
			else if(v >= (ANALOG_SCALED_MAX - Threshold)) v = ANALOG_SCALED_MAX;
		} 
		d = v - oldValue[chan];
		if(d < 0) d = -d;
		
		count++;
		
		if(d <= Threshold) continue;
		//if(d > Threshold) {
			/*if(scale & 1) {
				if(v <= Threshold ) v = 0;
				else if(v >= (4095 - Threshold)) v = 4095;
			}*/
		oldValue[chan] = v;
		//count++;
		
		if(mode & AMODE_CROSS) {
			if(Dist[chan] != 0) {
				d = v - inValue[chan];
				if((d != 0) && ((d > 0) == (Dist[chan] > 0))) { 
					Dist[chan] = d;
					continue; 
				}
				else Dist[chan] = 0;
			}
		}
		
		if(mode & AMODE_NUM) {
			if( !len ) buf[len++] = 'B';
			buf[len++] = chan + 100;
			buf[len++] = v >> 8;
			buf[len++] = v & 255;
		}
		else printf("C A %d %d\n",chan,v);
	}
	
	if((len != 0) && (mode & AMODE_NUM)) {
		buf[len++] = '\n';
		fraiseSend(buf,len);
	}
	
	return count;		
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

void analogInput()
{
}

void analogScaling(unsigned char scaling) // when scaling, min and max are updated each sample
{
	unsigned char i;
	
	if(scaling==1) {
		for(i=0;i<MAX_CHANNELS;i++) {
			Min[i]=0x7FFF;
			Max[i]=0;
		}
	}
	
	Scaling=scaling;
}

void analogDeclareEE()
{
	EEdeclareInt(&Max[0]);
	EEdeclareInt(&Max[1]);
	EEdeclareInt(&Max[2]);
	EEdeclareInt(&Max[3]);
	EEdeclareInt(&Max[4]);
	EEdeclareInt(&Max[5]);
	EEdeclareInt(&Max[6]);
	EEdeclareInt(&Max[7]);

	EEdeclareInt(&Min[0]);
	EEdeclareInt(&Min[1]);
	EEdeclareInt(&Min[2]);
	EEdeclareInt(&Min[3]);
	EEdeclareInt(&Min[4]);
	EEdeclareInt(&Min[5]);
	EEdeclareInt(&Min[6]);
	EEdeclareInt(&Min[7]);
}

void analogPrintMin()
{
	printf("C asm %d %d %d %d %d\n",Min[0],Min[1],Min[2],Min[3],Min[4]);
}

void analogPrintMax()
{
	printf("C asM %d %d %d %d %d\n",Max[0],Max[1],Max[2],Max[3],Max[4]);
}

