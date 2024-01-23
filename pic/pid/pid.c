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
/*#include <stdlib.h>
#include <stdio.h>
#include <string.h>*/

/*#include <core.h>
#include <fraisedevice.h>
#include <eeparams.h>*/

#include "pid.h"


void pidInit(t_pid *Pid)
{
	Pid->Out=0;
	Pid->Last=Pid->Int=0;
	Pid->GainP=Pid->GainI=Pid->GainD=0;
	//Pid->MaxOut=
}

void pidReset(t_pid *Pid)
{
	Pid->Out=0;
	Pid->Last=Pid->Int=0;
}

void pidCompute(t_pid *Pid,int err)
{
	static t_pid P;
#define MAXINT 	(1L<<26) // maximum value for integration sum
#define MAXDELTA (1L<<16) // maximum value for diff term
	
	static long out,maxout,delta;
	
	memcpy(&P,Pid,sizeof(P));
	
	maxout=(long)P.MaxOut<<8;
	
	out = (long)err*P.GainP;
	 
	if(P.GainI) {
		if( ((err>0)&&(P.Int<MAXINT)&&(P.Out<maxout)) || ((err<0)&&(P.Int>(-MAXINT))&&(P.Out>-maxout)) )
			P.Int += err;	// clip P.Int to [-MAXINT-|err|,MAXINT+|err|] ; don't increase if Output is already saturated (anti-windup)
			//if(err == 0) P.Int -= (P.Int)/16;
		out+=(P.Int>>4)*P.GainI;
	}
		
	if(P.GainD) {
		delta=((long)err-P.Last);
		if(delta>=MAXDELTA) delta=MAXDELTA-1;
		else if(delta<-MAXDELTA) delta=-MAXDELTA;
		out+=(delta<<1)*P.GainD;
	}
	
	P.Last = err ;
	
	// clipping :
	if(out>maxout) P.Out=maxout;
	else if(out<-maxout) P.Out=-maxout;
	else P.Out = out;

	memcpy(Pid,&P,sizeof(P));
}


void pidDeclareEE(t_pid *Pid)
{
	EEdeclareChar(&Pid->GainP);
	EEdeclareChar(&Pid->GainI);
	EEdeclareChar(&Pid->GainD);
	EEdeclareInt(&Pid->MaxOut);
}

void pidInput(t_pid *Pid)
{
	unsigned char c,c2;
	unsigned int i = 0;
	
	c=fraiseGetChar();
	if(c == 254) {
		fraiseSendCopy();
		c2=fraiseGetChar();		
		switch(c2) {
			GETPARAM(0, Pid->GainP, i);
			GETPARAM(1, Pid->GainI, i);
			GETPARAM(2, Pid->GainD, i);
			GETPARAM(3, Pid->MaxOut, i);
		}
		printf("%d %d\n", c2, i);
	}
	else switch(c) {
		PARAM_CHAR(0, Pid->GainP); break;
		PARAM_CHAR(1, Pid->GainI); if(Pid->GainI == 0) Pid->Int = 0; break;
		PARAM_CHAR(2, Pid->GainD); break;
		PARAM_INT(3, Pid->MaxOut); break;
	}
}



