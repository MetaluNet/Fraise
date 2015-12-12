/*********************************************************************
 *
 *                DC motor library for Fraise pic18f  device
 *				
 *				
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  2014     Original.
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

/*#include <core.h>
#include <fraisedevice.h>
#include <eeparams.h>*/
#include "dcmot.h"


static long incDeltaT, lastIncTime, deltaT, actualSpeed, error;
static int pos,deltaPos;
int dcmot_v,dcmot_vabs;
t_dcmotVars dcmotVars;
t_dcmotVolVars dcmotVolVars;
static t_dcmotSetting S; 

void dcmotCompute(t_dcmot *mot)
{

#define MAXSPEED ((unsigned long)(1UL<<24))
	
	//done by macro
	/*__critical { 
		//memcpy(&dcmotVolVars,&(mot->VolVars),sizeof(dcmotVolVars));
		dcmotVolVars.lastIncTime = mot->VolVars.lastIncTime;
		dcmotVolVars.IncDeltaT = mot->VolVars.IncDeltaT;
		dcmotVolVars.Position = mot->VolVars.Position;
		dcmotVolVars.flags = mot->VolVars.flags;
		mot->VolVars.IncDeltaT = 0;
	}*/

	memcpy(&dcmotVars,&(mot->Vars),sizeof(dcmotVars));
	memcpy(&S,&(mot->Setting),sizeof(S));
		
	if((S.Mode == 1) || (S.Mode == 3)) {
		deltaPos = dcmotVolVars.Position - dcmotVars.lastPosition;
		dcmotVars.lastPosition = dcmotVolVars.Position;

		if(!deltaPos) {
			deltaT = elapsed(dcmotVolVars.lastIncTime);
			if(!dcmotVars.stalled) {
				if((deltaT>>18) > S.StallTime) dcmotVars.stalled = 1;	// more than (8<<18)/2 us = 1 sec -> speed=0
			}
			if(!dcmotVars.stalled) {
				if(deltaT < dcmotVars.lastIncDeltaT) deltaT = dcmotVars.lastIncDeltaT; // 
				deltaPos = dcmotVolVars.direction ? 1 : -1; //make like a new step was occuring now, at the same rate the previous one.
			}
		}
		else {
			dcmotVars.stalled = 0;
			deltaT = dcmotVolVars.IncDeltaT;
			dcmotVars.lastIncDeltaT = deltaT;
		}
		
		if(deltaT <= 0) actualSpeed = 0;
		else actualSpeed = deltaPos * (MAXSPEED/((unsigned long)(deltaT)));
	}
	
	if((S.Mode >= 2) && (dcmotVolVars.homed)) {
		rampCompute(&mot->PosRamp);

		error = (long)(mot->PosRamp.currentPos>>(RAMP_UINCPOW)) - (dcmotVolVars.Position) ;

		if((error < 0) && (error >= -S.PosWindow)) error = 0;
		if((error > 0) && (error <= S.PosWindow)) error = 0;

		error <<= S.PosErrorGain; //7;
		if(error >= (1L<<15)) error = (1L<<15)-1;
		else if(error < -(1L<<15)) error = -(1L<<15);

		pidCompute(&mot->PosPID,(int)error);
		
		if(S.Mode == 3) dcmotVars.SpeedConsign = mot->PosPID.Out >> 8;
		else dcmotVars.PWMConsign = mot->PosPID.Out >> 8;
	}
	
	if((S.Mode == 1) || (S.Mode == 3)) {
		error = (long)(dcmotVars.SpeedConsign) - (long)actualSpeed;
	
		if(error >= (1L<<15)) error = (1L<<15)-1;
		else if(error < -(1L<<15)) error = -(1L<<15);
		
		pidCompute(&mot->SpeedPID, (int)error);
	
		dcmotVars.PWMConsign = mot->SpeedPID.Out>>8;
	}
	memcpy(&(mot->Vars),&dcmotVars,sizeof(dcmotVars));
}


void dcmotInput(t_dcmot *mot)
{
	unsigned char c,c2;
	unsigned int i = 0;

	c=fraiseGetChar();
	if(c == 254) {
		fraiseSendCopy();
		c2=fraiseGetChar();		
		switch(c2) {
			GETPARAM(3, mot->Vars.SpeedConsign, i);
			GETPARAM(4, mot->Vars.PWMConsign, i);
			GETPARAM(5, (char)mot->Setting.reversed, i);
		}
		printf("%d %d\n", c2, i);
	}
	else switch(c) {
		case 0 : rampInput(&mot->PosRamp); mot->Setting.Mode = 2; break;
		case 1 : pidInput(&mot->PosPID); break;
		case 2 : pidInput(&mot->SpeedPID); break;
		PARAM_INT(3, mot->Vars.SpeedConsign); mot->Setting.Mode = 1; break;
		PARAM_INT(4, mot->Vars.PWMConsign); mot->Setting.Mode = 0; break;
		PARAM_CHAR(5, mot->Setting.reversed); break;
		PARAM_CHAR(6, c2); mot->VolVars.homed = (c2!=0); if(c2==1) {
				mot->VolVars.Position = 0;
				rampSetPos(&mot->PosRamp,0);
			}
			break;
	}
}

void dcmot_declareEE(t_dcmot *mot)
{
	EEdeclareChar(&mot->Setting.reversed);
	rampDeclareEE(&mot->PosRamp);
	pidDeclareEE(&mot->PosPID);
	pidDeclareEE(&mot->SpeedPID);
}


