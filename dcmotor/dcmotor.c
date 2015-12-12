/*********************************************************************
 *
 *                DCMotor library for Fraise pic18f  device
 *				-Ramp generator with maxspeed/acceleration
 *				-PID regulator with maximum output setting
 *					and anti-windup tracking gain
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  april 2012     Original.
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <core.h>
#include <fraisedevice.h>
#include <eeparams.h>

#include <dcmotor.h>


void DCMInit(t_DCMotor *Motor)
{
	Motor->Status.val=0;
	/*Motor->Pos=Motor->DestPos=0;
	Motor->ConsignPos=0;
	Motor->MaxCurrent=0;
	Motor->MaxOut=0;
	Motor->Int=0;
	Motor->GainP=Motor->GainI=Motor->GainD=Motor->GainT=0;*/
}

/*void DCMSetPos(t_DCMotor *Motor,int pos)
{
	Motor->DestPos=Motor->Pos=Motor->OldPos=pos;
	Motor->ConsignPos=((long)pos)<<UINCPOW;
	Motor->Speed=0;
	Motor->Int=0;
	Motor->PIDOut=0;
	Motor->Status.enable=0;
	Motor->Status.homed=1;
}

void DCMGoto(t_DCMotor *Motor,int pos)
{
	Motor->DestPos=pos;
	Motor->StaticCount=0;
}


void DCMCompute(t_DCMotor *Motor)
{
	static t_DCMotor M;

	int d,ds,absspeed;
	long int absspeed_l,maxspeed_l;
	
	long err;//PID input error
	long out;
	int intconsignpos;
	
	memcpy(&M,Motor,sizeof(M));
	
	absspeed_l=M.Speed;if(absspeed_l<0) absspeed_l=-absspeed_l;
	
	//d=(M.DestPos-(int)((M.ConsignPos)/4096));
	d=(M.DestPos-(int)((M.ConsignPos)>>UINCPOW));
	//if((d>-3)&&(d<3)&&((absspeed_l/2)<=M.MaxAccel)) { //target proximity
	if((d>-3)&&(d<3)&&((absspeed_l>>1)<=M.MaxAccel)) { //target proximity
		//M.ConsignPos=((long int)M.DestPos)*4096;
		M.ConsignPos=((long int)M.DestPos)<<UINCPOW;
		M.Speed=0;
	}
	else {
		//absspeed=1+(absspeed_l/4096);
		absspeed=1+(absspeed_l>>UINCPOW);
		//ds=((long)absspeed/2*absspeed)/(M.MaxAccel);
		ds=(((long)absspeed>>1)*absspeed)/(M.MaxAccel);
		if(M.Speed<0) ds=-ds;

		if(d>ds) M.Speed+=M.MaxAccel;
		else M.Speed-=M.MaxAccel;

		//maxspeed_l=((long)M.MaxSpeed)*4096;
		maxspeed_l=((long)M.MaxSpeed)<<UINCPOW;
		if(M.Speed>maxspeed_l) M.Speed=maxspeed_l;
		else if((-M.Speed)>maxspeed_l) M.Speed=-maxspeed_l;

		//M.ConsignPos+=(M.Speed/4096);
		M.ConsignPos+=(M.Speed>>UINCPOW);
		M.DS=ds;
	}
	
	//if(M.Speed!=0) M.StaticCount=0;
	
	if(M.StaticCount>3000) { // if static for more than 6 sec (3000*2ms)
		//M->Olderr=0;
		M.StaticCount=3001;
		M.Int=0;
		M.Status.enable=0;
		M.PIDOut=0;
		//UpdateMotor2();
		goto dcmcend;
	}
	
	M.Status.enable=1;
	if(M.OldPos==M.Pos) M.StaticCount++;
	else M.StaticCount=0;
	M.OldPos=M.Pos;
	
	//intconsignpos=M.ConsignPos/4096;
	intconsignpos=M.ConsignPos>>UINCPOW;
	err=intconsignpos-M.Pos;
	//derr=err-Olderr2;
	//Olderr2=err;

	//M.Int+=(err*(long)M.GainI)/(long)16384;
	//out=err*M.GainP+M.Int;//(M.Int*M.GainI)/16384;//+(derr*GainD2*16);

	M.Int+=err;
	//out=err*M.GainP+(M.Int*M.GainI)/16384;//+(derr*GainD2*16);
//	out=err*M.GainP+((M.Int*M.GainI)>>14);
	out=err*M.GainP+((M.Int*M.GainI)>>(UINCPOW+2));
	
	if(out>M.MaxOut) {
		M.PIDOut=M.MaxOut;
		//M.Int-=((out-M.MaxOut)*M.GainT)/256; //anti windup
		M.Int-=((out-M.MaxOut)*M.GainT)>>8; //anti windup
	}
	else if((-out)>M.MaxOut) {
		M.PIDOut=-M.MaxOut;
		M.Int-=((out+M.MaxOut)*M.GainT)>>8; //anti windup
	}
	else M.PIDOut=out;	
	//UpdateMotor2();
dcmcend:
	memcpy(Motor,&M,sizeof(M));

}*/

/*static int eeaddress;

#define EE_READBYTE_NEXT() (ee_read_byte((char)(eeaddress++)))
#define EE_WRITEBYTE_NEXT(data) ee_write_byte((char)(eeaddress++),data)

static char EELoadChar()
{
	return EE_READBYTE_NEXT();
}

static int EELoadInt()
{
	return (EE_READBYTE_NEXT()<<8)|EE_READBYTE_NEXT();
}

static void EESaveChar(unsigned char data)
{
	EE_WRITEBYTE_NEXT(data);
}

static void EESaveInt(int data)
{
	EE_WRITEBYTE_NEXT(data>>8);
	EE_WRITEBYTE_NEXT(data&255);
}*/

/*int DCMLoadEE(t_DCMotor *Motor,int address)
{
	eeaddress=address;
	Motor->Status.val=EELoadChar();
	Motor->MaxCurrent=EELoadInt();
	Motor->MaxSpeed=EELoadInt();
	Motor->MaxAccel=EELoadInt();
	Motor->GainP=EELoadChar();
	Motor->GainI=EELoadChar();
	Motor->GainD=EELoadChar();
	Motor->GainT=EELoadChar();
	Motor->MaxOut=EELoadInt();
	Motor->Int=0;
	return eeaddress;
}

int DCMSaveEE(t_DCMotor *Motor,int address)
{
	eeaddress=address;
	EESaveChar(Motor->Status.val);
	EESaveInt(Motor->MaxCurrent);
	EESaveInt(Motor->MaxSpeed);
	EESaveInt(Motor->MaxAccel);
	EESaveChar(Motor->GainP);
	EESaveChar(Motor->GainI);
	EESaveChar(Motor->GainD);
	EESaveChar(Motor->GainT);
	EESaveInt(Motor->MaxOut);
	return eeaddress;
}*/

/*#define PARAM_CHAR(n,p) case n: p=FrRXgetchar();
//#define PARAM_INT(n,p) case n: p=FrRXgetchar()<<8;p+=FrRXgetchar();
#define PARAM_INT(n,p) case n: p=((unsigned int)FrRXgetchar()<<8) | (unsigned int)FrRXgetchar();
#define PARAM_LONG(n,p) case n: p=FrRXgetchar()<<24;p+=FrRXgetchar()<<16;p+=FrRXgetchar()<<8;p+=FrRXgetchar();

#define GETPARAM(n,p) case n: i=p; break*/
void DCM_declareEE(t_DCMotor *Motor)
{
	EEdeclareChar(&Motor->Status.val);
	/*EEdeclareInt(&Motor->MaxCurrent);
	EEdeclareInt(&Motor->MaxSpeed);
	EEdeclareInt(&Motor->MaxAccel);
	EEdeclareChar(&Motor->GainP);
	EEdeclareChar(&Motor->GainI);
	EEdeclareChar(&Motor->GainD);
	EEdeclareChar(&Motor->GainT);
	EEdeclareInt(&Motor->MaxOut);*/
}

void DCMInput(t_DCMotor *Motor,unsigned char fraddress)
{
	unsigned char c,c2;
	int i=0;
	
	c=FrRXgetchar();
	switch(c) {
		case 8: Motor->Status.reversed=(FrRXgetchar()!=0); /*Motor->Int=0;*/ break;
		//case 9: Motor->Status.analog=(FrRXgetchar()!=0); break;
		case 254: c2=FrRXgetchar();
			switch(c2) {
				/*GETPARAM(0,Motor->MaxCurrent);
				GETPARAM(1,Motor->MaxSpeed);
				GETPARAM(2,Motor->MaxAccel);
				GETPARAM(3,Motor->GainP);
				GETPARAM(4,Motor->GainI);
				GETPARAM(5,Motor->GainD);
				GETPARAM(6,Motor->GainT);
				GETPARAM(7,Motor->MaxOut);*/
				GETPARAM(8,Motor->Status.reversed);
				/*GETPARAM(9,Motor->Status.analog);
				GETPARAM(100,Motor->DestPos);*/
			}
			printf("C %d %d %d\n",fraddress,c2,i);
			break;
		//case 255: DCMSaveEE(Motor,eeaddress);break;
	}
	
	/*if(!Motor->Status.homed) return;
	
	switch(c) {
		PARAM_INT(0,Motor->MaxCurrent); break;
		PARAM_INT(1,Motor->MaxSpeed); break;
		PARAM_INT(2,Motor->MaxAccel); break;
		PARAM_CHAR(3,Motor->GainP); break;
		PARAM_CHAR(4,Motor->GainI); Motor->Int=0;break;
		PARAM_CHAR(5,Motor->GainD); break;
		PARAM_CHAR(6,Motor->GainT); break;
		PARAM_INT(7,Motor->MaxOut); break;
		PARAM_INT(100,i);DCMGoto(Motor,i);break;
	}*/
}



