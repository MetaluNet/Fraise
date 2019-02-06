/*********************************************************************
 *
 *                pid library for Fraise pic18f  device
 *
 *				-pid regulator with maximum output setting
 *					and anti-windup tracking gain
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
#ifndef _pid_H_
#define _pid_H_

#include <fruit.h>

/*

error input : signed 16 bit (int) 
gains : unsigned 8 bits

Int = Integration sum clipped to [-1<<26, 1<<26[, then divided by 16 (>>4)
Integration sum of error*GainI : signed max 31 bits

Proportionnal term error*GainP : signed max 24 bits

GainD is multiplied by 128 (1<<7)
Differential term derror*GainD : signed max 30 bits
 

*/
typedef struct {
	unsigned char GainP, GainI, GainD;
	long int Int;		// integral sum
	long int Last;		// last error, used to compute differential term
	long int Out;		// output of pid regulator
	int MaxOut;		// maximum ouput allowed for pid regulator ; multiplied by 256.
} t_pid ;

void pidInit(t_pid *Pid);
void pidReset(t_pid *Pid);
void pidCompute(t_pid *Pid,int err);
void pidInput(t_pid *Pid);
void pidDeclareEE(t_pid *Pid);
#define pid_EESIZE 5 // 5 bytes of eeprom

#endif // _pid_H_

