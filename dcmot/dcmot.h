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
#ifndef _DCMOT_H_
#define _DCMOT_H_

#include <fruit.h>
#include <pid.h>
#include <ramp.h>

typedef union{
	unsigned char val;
	struct {
		unsigned incA:1; 
		unsigned incB:1;
		unsigned lastA:1; 
		unsigned lastB:1;
	};
} t_dcmotIncr;

typedef struct {
	int SpeedConsign;
	int PWMConsign;
	long int lastIncDeltaT; 	// interval between incs on last Compute() call
	long int lastPosition; 		// position on last Compute() call
	unsigned stalled:1; 		// the motor doesn't run anymore
} t_dcmotVars;

typedef struct {
	long int lastIncTime; 	// last inc event time
	long int IncDeltaT; 	// interval between last captured inc and the one captured just before the last reset.
										// Must be reset in critical section, when reading Position.
	long int Position;
	union{
		unsigned char flags;
		struct {
			unsigned homed:1 ; 					//homing done : end switch has been detected.
			unsigned direction:1; 				// last measured direction ; 0=position_decrease 1=position_increase
			unsigned end:1;
		};
	};	
} t_dcmotVolVars;

typedef struct {
	unsigned char Mode ; 		//0:pwm 1:speed 2:pos 3:pos+speed_regulator
	 int PosWindow; 	// tolerated absolute position error
	 int PwmMin; 		// minimum absolute pwm value
	unsigned char StallTime;	// if position has not changed during this period, the motor is considered stopped. unit = 1/8 s.
	unsigned char PosErrorGain;	// position error multiplier before PosPID
	unsigned char reversed; 		//1 if positive speed decreases measured position
} t_dcmotSetting;

typedef struct {
	t_dcmotSetting Setting; 
	t_dcmotIncr Incr;
	t_dcmotVars Vars;
	volatile t_dcmotVolVars VolVars;
	
	t_ramp PosRamp;
	t_pid PosPID;  	//position pid
	t_pid SpeedPID; //speed pid 
} t_dcmot ;

extern int dcmot_v,dcmot_vabs;
extern t_dcmotVars dcmotVars;
extern t_dcmotVolVars dcmotVolVars;

#define SET_PWM_(pwm,val) do{ CCP##pwm##CONbits.DC##pwm##B1 = val&2;  CCP##pwm##CONbits.DC##pwm##B0 = val&1; CCPR##pwm##L=val>>2; } while(0)

#define SET_PWM(pwm,val) CALL_FUN2(SET_PWM_, pwm, val)


#define DCMOT_DECLARE_(motID) t_dcmot dcmot##motID
#define DCMOT_DECLARE(motID) CALL_FUN(DCMOT_DECLARE_,motID)

#define DCMOT_CAPTURE_SERVICE_(motID) do{ 						\
	dcmot##motID.Incr.incA = digitalRead(MOT##motID##_A); 		\
	if(!digitalRead(MOT##motID##_END)) { 						\
		dcmot##motID.VolVars.Position = 0; 						\
		dcmot##motID.VolVars.homed = 1;							\
		dcmot##motID.VolVars.end = 1;							\
	}	else dcmot##motID.VolVars.end = 0;						\
	if(dcmot##motID.Incr.incA != dcmot##motID.Incr.lastA) { 	\
		dcmot##motID.Incr.lastA = dcmot##motID.Incr.incA;		\
		dcmot##motID.VolVars.IncDeltaT -= dcmot##motID.VolVars.lastIncTime;		\
		dcmot##motID.VolVars.lastIncTime = timeISR();		\
		dcmot##motID.VolVars.IncDeltaT += timeISR();			\
		dcmot##motID.Incr.incB = digitalRead(MOT##motID##_B); 	\
		if(dcmot##motID.Incr.incA ^ !dcmot##motID.Incr.incB) {	\
			dcmot##motID.VolVars.Position++;					\
			dcmot##motID.VolVars.direction = 1;					\
		}														\
		else  {													\
			dcmot##motID.VolVars.Position--;					\
			dcmot##motID.VolVars.direction = 0;					\
		}														\
	}															\
 } while(0)	
#define DCMOT_CAPTURE_SERVICE(motID) CALL_FUN(DCMOT_CAPTURE_SERVICE_,motID)

#define DCMOT_CAPTURE_SERVICE_SINGLE_(motID) do{ 				\
	dcmot##motID.Incr.incA = digitalRead(MOT##motID##_A); 		\
	if(!digitalRead(MOT##motID##_END)) { 						\
		dcmot##motID.VolVars.Position = 0; 						\
		dcmot##motID.VolVars.homed = 1;							\
		dcmot##motID.VolVars.end = 1;							\
	}	else dcmot##motID.VolVars.end = 0;						\
	if(dcmot##motID.Incr.incA != dcmot##motID.Incr.lastA) { 	\
		dcmot##motID.Incr.lastA = dcmot##motID.Incr.incA;		\
		dcmot##motID.VolVars.IncDeltaT -= dcmot##motID.VolVars.lastIncTime;		\
		dcmot##motID.VolVars.lastIncTime = timeISR();		\
		dcmot##motID.VolVars.IncDeltaT += timeISR();			\
		dcmot##motID.Incr.incB = digitalRead(MOT##motID##_B); 	\
		if(dcmot##motID.VolVars.direction) dcmot##motID.VolVars.Position++;		\
		else  dcmot##motID.VolVars.Position--;					\
	}															\
 } while(0)	
#define DCMOT_CAPTURE_SERVICE(motID) CALL_FUN(DCMOT_CAPTURE_SERVICE_,motID)


#define DCMOT_INIT_(motID) do{\
	/*SETPORT_MOT##motID;*/\
	pinModeDigitalOut(M##motID##1);\
	pinModeDigitalOut(M##motID##2);\
	pinModeDigitalOut(M##motID##EN);\
	SET_PWM(MOT##motID##_PWM, 0);\
	/*MOT##motID##_IN1 = 0;*/digitalClear(M##motID##1);\
	/*MOT##motID##_IN2 = 0;*/digitalClear(M##motID##2);\
	/*MOT##motID##_EN = 1;*/digitalSet(M##motID##EN);\
	pinModeDigitalIn(MOT##motID##_END);\
	pinModeDigitalIn(MOT##motID##_A);\
	pinModeDigitalIn(MOT##motID##_B);\
	dcmot##motID.Setting.Mode = 0;\
	dcmot##motID.Setting.PosWindow = 2;\
	dcmot##motID.Setting.PwmMin = 10;\
	dcmot##motID.Setting.StallTime = 16;\
	dcmot##motID.Setting.PosErrorGain = 7;\
	dcmot##motID.Setting.reversed = 0;\
	rampInit(&dcmot##motID.PosRamp);\
	pidInit(&dcmot##motID.SpeedPID);\
	pidInit(&dcmot##motID.PosPID);\
	dcmot##motID.VolVars.Position = 0;\
	dcmot##motID.VolVars.homed = 0;\
	dcmot##motID.VolVars.end = 0;\
	dcmot##motID.Vars.PWMConsign = dcmot##motID.Vars.SpeedConsign = 0;\
	dcmot##motID.Vars.lastPosition = 0;\
 } while(0)	
#define DCMOT_INIT(motID) CALL_FUN(DCMOT_INIT_,motID)

#define DCMOT_LOAD_dcmotVolVars_(motID) do{ \
	__critical {\
		dcmotVolVars.lastIncTime = dcmot##motID.VolVars.lastIncTime;\
		dcmotVolVars.IncDeltaT = dcmot##motID.VolVars.IncDeltaT;\
		dcmotVolVars.Position = dcmot##motID.VolVars.Position;\
		dcmotVolVars.flags = dcmot##motID.VolVars.flags;\
		dcmot##motID.VolVars.IncDeltaT = 0;\
	}\
 } while(0)

void dcmotCompute(t_dcmot *mot);

#define DCMOT_FORMATPWM(motID) do{ \
	dcmot_v = dcmot##motID.Vars.PWMConsign; \
	if(dcmot_v > 1023) dcmot_v = 1023;	\
	if(dcmot_v < -1023) dcmot_v = -1023;	\
	if((dcmot_v > 0) && (dcmot_v < dcmot##motID.Setting.PwmMin)) dcmot_v = 0; \
	if((dcmot_v < 0) && (dcmot_v > -dcmot##motID.Setting.PwmMin)) dcmot_v = 0; \
	if(dcmot##motID.VolVars.end && (dcmot_v < 0)) dcmot_v = 0;\
	/*dcmot_v  = (dcmot##motID.Setting.reversed ? -dcmot_v : dcmot_v);*/\
} while(0)

#define DCMOT_UPDATE_ASYM_(motID) do{ \
	DCMOT_FORMATPWM(motID);\
	dcmot_vabs = dcmot_v < 0 ? 1023 + dcmot_v : dcmot_v; \
	SET_PWM(MOT##motID##_PWM, dcmot_vabs); \
	if(dcmot_v < 0) { digitalSet(M##motID##2);}\
	else { digitalClear(M##motID##2);}\
 } while(0)
#define DCMOT_UPDATE_ASYM(motID) CALL_FUN(DCMOT_UPDATE_ASYM_,motID)

#define DCMOT_UPDATE_SYM_(motID) do{ \
	DCMOT_FORMATPWM(motID);\
	dcmot_vabs = dcmot_v < 0 ? -dcmot_v : dcmot_v; \
	SET_PWM(MOT##motID##_PWM, dcmot_vabs); \
	if(dcmot_v < 0) { digitalClear(M##motID##1);digitalSet(M##motID##2);}\
	else { digitalClear(M##motID##2); digitalSet(M##motID##1);}\
 } while(0)
#define DCMOT_UPDATE_SYM(motID) CALL_FUN(DCMOT_UPDATE_SYM_,motID)

#define DCMOT_COMPUTE(motID, mode) do{\
	DCMOT_LOAD_dcmotVolVars_(motID);\
	dcmotCompute(&dcmot##motID);\
	DCMOT_UPDATE_##mode(motID);\
 } while(0)

#define DCMOT_SETDIR_(motID) do{ \
	if(dcmot_v < 0) dcmot##motID.VolVars.direction = 0;\
	else dcmot##motID.VolVars.direction = 1;\
 } while(0)
 
#define DCMOT_COMPUTE_SINGLE(motID, mode) do{\
	DCMOT_COMPUTE(motID, mode);\
	DCMOT_SETDIR_(motID);\
 } while(0)

void dcmotInput(t_dcmot *mot);
#define DCMOT_INPUT(motID) dcmot_input(&dcmot##motID)

void dcmotDeclareEE(t_dcmot *mot);
#define DCMOT_DECLARE_EE(motID) do{ dcmot_declareEE(&dcmot##motID);} while(0)

#define DCMOT(motID) (dcmot##motID)
#define DCMOT_GETPOS(motID) (DCMOT(motID).VolVars.Position)

#endif // _DCMOT_H_
