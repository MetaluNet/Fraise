/*********************************************************************
 *
 *                Ramp library for Fraise pic18f  device
 *
 *  Ramp generator with destination position, maxspeed/acceleration
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
#ifndef _ramp_H_
#define _ramp_H_

#include <fruit.h>

#ifndef RAMP_UINCPOW
#define RAMP_UINCPOW 10 // 1 increment = 1024 milli-increments (mincs) = 1024x1024 micro-increments (uincs)
#endif

#ifndef RAMP_MAXERROR 
#define RAMP_MAXERROR 3 //  
#endif


typedef struct {
	int destPos; //Destination position, in incs
	long int currentPos; //Current position in mincs
	int maxSpeed; 	//Maximum absolute speed allowed to ramp generator, in minc/ms
	long int speed; //Consign speed computed by ramp generator, in uinc/ms
	int maxAccel ;  //Maximum acceleration allowed to ramp generator, in uinc/ms/ms

					//ex: RAMP_UINCPOW=10
					// maxAccel=1 -> 1 uinc/ms/ms : during 1s, speed will be increased of 1000/1024 minc/ms : 
					// it will take about 1s for speed to increase of 1 maxSpeed unity.   
					// maxAccel=1024 -> 1uinc/ms/ms:1000(uinc/ms)/s: 1.024s to reach speed 1024 (=1 inc/s)
					// max 65535-> about 1/16s  to reach speed 1024

	int maxDecel ;  //Maximum acceleration allowed to trajectory generator, in uinc/ms/ms
} t_ramp ;

void rampInit(t_ramp *Ramp);
void rampGoto(t_ramp *Ramp,int pos);
void rampSetPos(t_ramp *Ramp,int pos);
#define rampGetPos(Ramp) ((Ramp)->currentPos>>(RAMP_UINCPOW))
void rampCompute(t_ramp *Ramp);
void rampInput(t_ramp *Ramp);
void rampDeclareEE(t_ramp *Ramp);

#define RAMP_EESIZE 6 // 4 bytes of eeprom

#endif // _PID_H_
