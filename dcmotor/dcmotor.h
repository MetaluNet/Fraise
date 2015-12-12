/*********************************************************************
 *
 *                DCMotor library for Fraise pic18f device
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
#ifndef _DCMOTOR_H_
#define _DCMOTOR_H_

#include <config.h>

#ifndef UINCPOW
#define UINCPOW 12
#endif

typedef union{
	unsigned char val;
	struct {
		unsigned reversed:1; //1 if positive speed decreases measured position
		unsigned enable:1;
		unsigned homed:1 ; //homing done
		unsigned analog:1; //position is analogicaly measured (potentiometer) or numerical (incremental)
	};
} t_DCMotorStatus ;


typedef struct {
	t_DCMotorStatus Status;
	/*unsigned int MaxCurrent;
	int Pos; //Measured position, in "inc" (increment)
	int DestPos; //Destination position, in "inc"
	long int ConsignPos; //Fractionnal consign position computed by trajectory generator.
						//4096 times greater than "inc" ( 1 inc=4096 uinc("microinc") )
	int MaxSpeed; 	//Maximum absolute speed allowed to trajectory generator, in uinc/ms
						//ex: 1->1uinc/ms=1000/4096 inc/s ; 4096->1000inc/s
	long int Speed; //Consign speed computed by trajectory generator, in 1/4096 uinc/ms
					//(4096 times greater than MaxSpeed unity)
	int MaxAccel ;  //Maximum acceleration allowed to trajectory generator, 1/4096 uinc/ms/ms
					//ex: 1->1/4096uinc/ms/ms : during 1s, speed will be increased of 1000/4096 uinc/ms : 
					// it will take 4s for speed to increase of 1 MaxSpeed unity.   
					//    4096->1uinc/ms/ms:1000(uinc/ms)/s: 4.096s to reach speed 4096
					// max 65535-> about 1/4s  to reach speed 4096
	unsigned char GainP, GainI, GainD; //PID gains
	unsigned char GainT; //Intergal term anti-windup tracking gain
	long int Int; //integral sum
	int PIDOut; //output of pid regulator
	int MaxOut; //maximum ouput allowed for PID regulator
	int OldPos;
	int StaticCount ; //how much time since motor really moved
	int DS; //debug : (algebric) shortest distance before stop*/
} t_DCMotor ;

void DCMInit(t_DCMotor *Motor);
/*void DCMGoto(t_DCMotor *Motor,int pos);
void DCMSetPos(t_DCMotor *Motor,int pos);
void DCMCompute(t_DCMotor *Motor);*/
/*int DCMLoadEE(t_DCMotor *Motor,int address);
int DCMSaveEE(t_DCMotor *Motor,int address);*/
void DCMInput(t_DCMotor *Motor,unsigned char fraddress);
void DCM_declareEE(t_DCMotor *Motor);

#endif // _DCMOTOR_H_

