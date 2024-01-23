/*********************************************************************
 *
 *                Ramp library for Fraise pic18f  device
 *
 *				Ramp generator with destination position, maxspeed/acceleration
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  dec 2012     Original.
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
/*#include <stdlib.h>
#include <stdio.h>
#include <string.h>*/

/*#include <core.h>
#include <fraisedevice.h>
#include <eeparams.h>*/

//#include <config.h>
#include "ramp.h"


void rampInit(t_ramp *Ramp)
{
	Ramp->destPos = 0;
	Ramp->currentPos = 0;
	Ramp->speed = 0;
	Ramp->length = 0;
}

void rampSetPos(t_ramp *Ramp, long int pos)
{
	if(Ramp->length) pos = (pos + Ramp->length)% Ramp->length;
	Ramp->destPos = pos;
	Ramp->currentPos = ((long)pos) << RAMP_UINCPOW;
	Ramp->speed = 0;
}

void rampSetPosMoving(t_ramp *Ramp, long int pos)
{
	if(Ramp->length) pos = (pos + Ramp->length) % Ramp->length;
	Ramp->destPos = pos;
	Ramp->currentPos = ((long)pos) << RAMP_UINCPOW;
}

void rampGoto(t_ramp *Ramp,long int pos)
{
	if(Ramp->length) pos = (pos + Ramp->length) % Ramp->length;
	Ramp->destPos = pos;
}

void rampMove(t_ramp *Ramp, long int offset)
{
	long int pos = Ramp->destPos + offset;
	if(Ramp->length) pos = (pos + Ramp->length) % Ramp->length;
	Ramp->destPos = pos;
}

void rampCompute(t_ramp *Ramp)
{
	static t_ramp M;

	int ds, absspeed, absspeed_tmp;
	long d;
	long int absspeed_l, maxspeed_l;

	memcpy(&M, Ramp, sizeof(M));

	absspeed_l = M.speed;
	if(absspeed_l < 0) absspeed_l = -absspeed_l;
	
	d = ((long)M.destPos - (long)(M.currentPos >> RAMP_UINCPOW));
	if(M.length) d = ((d + M.length + (M.length / 2)) % M.length) - (M.length / 2);

	if((d > -RAMP_MAXERROR) && (d < RAMP_MAXERROR) && ((absspeed_l >> 1) <= M.maxDecel)) { //target proximity
		M.currentPos = ((long int)M.destPos) << RAMP_UINCPOW;
		M.speed = 0;
	}
	else {
		absspeed = absspeed_l >> (RAMP_VPOW + 1);
		absspeed_tmp = absspeed_l >> RAMP_UINCPOW;
		ds = (((long)(absspeed) * absspeed_tmp) / M.maxDecel); //stop distance = speed²/2d
		//ds=ds+ds>>1; //over evaluate ds...
		
		maxspeed_l = ((long)M.maxSpeed) << RAMP_VPOW;
		
		#if 1
		if(M.speed >= 0) {
			if(d > ds) {
				if(M.speed < maxspeed_l) {
					M.speed += M.maxAccel;
					if(M.maxDecel < M.maxAccel){
						//verify :
						absspeed = M.speed >> (RAMP_VPOW + 1);
						absspeed_tmp = M.speed >> RAMP_UINCPOW;
						ds= (((long)(absspeed) * absspeed_tmp) / M.maxDecel); //stop distance = speed²/2d
						//ds = ds + ds >> 1;//over evaluate ds...
						if(d < ds) M.speed -= M.maxAccel - M.maxDecel;
					}
					if(M.speed > maxspeed_l) M.speed = maxspeed_l;
				}
				else if(M.speed > maxspeed_l) M.speed -= M.maxDecel;
			}
			else M.speed -= ((long)M.maxDecel) + (long)M.maxAccel;//M.maxDecel<<1; //really don't know why i have to apply decel twice, to avoid overshoot...
		}
		else {
			ds = -ds;
			if(d < ds) {
				if(M.speed > -maxspeed_l) {
					M.speed -= M.maxAccel;
					if(M.maxDecel < M.maxAccel){
						//verify :
						absspeed = -(M.speed >> (RAMP_VPOW + 1));
						absspeed_tmp = -(M.speed >> RAMP_UINCPOW);
						ds = -(((long)(absspeed) * absspeed_tmp) / (M.maxDecel)); //stop distance = speed²/2d
						//ds=ds+ds>>1;//over evaluate ds...
						if(d > ds) M.speed += M.maxAccel - M.maxDecel;
					}
					if(M.speed < -maxspeed_l) M.speed = -maxspeed_l;
				}
				else if(M.speed < -maxspeed_l) M.speed += M.maxDecel;
			}
			else M.speed += (long)M.maxDecel + (long)M.maxAccel;//M.maxDecel<<1; //really don't why i have to apply decel twice, to avoid overshoot...
		}
		#else
		if(M.speed < 0) ds = -ds;

		if(d>ds) M.speed+=M.maxDecel;
		else M.speed-=M.maxDecel;
		
		/*if(M.speed<0) ds=-ds;

		if(d>ds) M.speed+=M.maxAccel;
		else M.speed-=M.maxAccel;*/

		if(M.speed>maxspeed_l) M.speed=maxspeed_l;
		else if((-M.speed)>maxspeed_l) M.speed=-maxspeed_l;
		#endif

		M.currentPos += (M.speed >> RAMP_VPOW);
	}

	if(M.length) //M.currentPos = (M.currentPos + (M.length << RAMP_UINCPOW)) % (M.length << RAMP_UINCPOW);
	{
		if(M.currentPos < 0) M.currentPos += (M.length << RAMP_UINCPOW);
		else if(M.currentPos >= (M.length << RAMP_UINCPOW)) M.currentPos -= (M.length << RAMP_UINCPOW);
	}
	
	memcpy(Ramp, &M, sizeof(M));

}

void rampInput(t_ramp *Ramp)
{
	unsigned char c,c2;
	int i = 0;
	long int l = 0;
	
	c=fraiseGetChar();
	if(c == 254) {
		fraiseSendCopy();
		c2=fraiseGetChar();
		switch(c2) {
			GETPARAM(0, Ramp->maxSpeed, l);
			GETPARAM(1, Ramp->maxAccel, l);
			GETPARAM(2, Ramp->maxDecel, l);
			GETPARAM(10, Ramp->destPos, l);
			GETPARAM(11, rampGetPos(Ramp), l);
			GETPARAM(13, Ramp->length, l);
		}
		printf("%d %ld\n", c2, l);
	} else switch(c) {
		PARAM_INT(0, Ramp->maxSpeed); break;
		PARAM_INT(1, Ramp->maxAccel); break;
		PARAM_INT(2, Ramp->maxDecel); break;
		PARAM_LONG(10, l); rampGoto(Ramp,l); break;
		PARAM_LONG(11, l); rampSetPos(Ramp,l); break;
		PARAM_LONG(12, l); rampSetPosMoving(Ramp,l); break;
		PARAM_LONG(13, l); Ramp->length = l; break;
	}
}

void rampDeclareEE(t_ramp *Ramp)
{
	EEdeclareInt(&Ramp->maxSpeed);
	EEdeclareInt(&Ramp->maxAccel);
	EEdeclareInt(&Ramp->maxDecel);
	EEdeclareLong(&Ramp->length);
}


