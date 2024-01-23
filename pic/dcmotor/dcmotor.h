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
#ifndef _DCMOTOR_H_
#define _DCMOTOR_H_

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
} t_dcmotorIncr;

typedef struct {
	int SpeedConsign;
	int PWMConsign;
	long int lastIncDeltaT; 	// interval between incs on last Compute() call
	long int lastPosition; 		// position on last Compute() call
	unsigned stalled:1; 		// the motor doesn't run anymore
} t_dcmotorVars;

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
} t_dcmotorVolVars;

typedef struct {
	unsigned char Mode ; 		//0:pwm 1:speed 2:pos 3:pos+speed_regulator
	int PosWindow; 	// tolerated absolute position error
	int PwmMin; 		// minimum absolute pwm value
	unsigned char StallTime;	// if position has not changed during this period, the motor is considered stopped. unit = 1/8 s.
	unsigned char PosErrorGain;	// position error multiplier before PosPID
	union{
		unsigned char flags;
		struct {
        	unsigned reversed:1; 		// 1 if positive speed decreases measured position
        	unsigned onlyPositive:1; 		// motor position is not allowed to be negative : when pos==0 only positive speed is allowed.
        };
    };
} t_dcmotorSetting;

typedef struct {
	t_dcmotorSetting Setting; 
	t_dcmotorIncr Incr;
	t_dcmotorVars Vars;
	volatile t_dcmotorVolVars VolVars;
	
	t_ramp PosRamp;
	t_pid PosPID;  	//position pid
	t_pid SpeedPID; //speed pid 
} t_dcmotor ;

extern int dcmotor_v,dcmotor_vabs;
extern t_dcmotorVars dcmotorVars;
extern t_dcmotorVolVars dcmotorVolVars;
// I really don't know why compiler complains if I use a 1 arg macro for INIT_PWM_...
#define INIT_PWM_(pwm,unused) do{CCP##pwm##CON = 0b00001100;} while(0) /* single PWM active high*/
#define INIT_PWM(pwm) CALL_FUN2(INIT_PWM_,pwm,0)

#define SET_PWM_(pwm,val) do{ CCP##pwm##CONbits.DC##pwm##B1 = val&2;  CCP##pwm##CONbits.DC##pwm##B0 = val&1; CCPR##pwm##L=val>>2; } while(0)

#define SET_PWM(pwm,val) CALL_FUN2(SET_PWM_, pwm, val)


#define DCMOTOR_DECLARE_(motID) t_dcmotor dcmotor##motID
#define DCMOTOR_DECLARE(motID) CALL_FUN(DCMOTOR_DECLARE_,motID)

#define DCMOTOR_CAPTURE_SERVICE_(motID) do{ 						\
	dcmotor##motID.Incr.incA = digitalRead(MOT##motID##_A); 		\
	if(digitalRead(MOT##motID##_END) == MOT##motID##_ENDLEVEL) { 	\
		dcmotor##motID.VolVars.Position = 0; 						\
		dcmotor##motID.VolVars.homed = 1;							\
		dcmotor##motID.VolVars.end = 1;								\
	}	else dcmotor##motID.VolVars.end = 0;						\
	if(dcmotor##motID.Incr.incA != dcmotor##motID.Incr.lastA) { 	\
		dcmotor##motID.Incr.lastA = dcmotor##motID.Incr.incA;		\
		dcmotor##motID.VolVars.IncDeltaT -= dcmotor##motID.VolVars.lastIncTime;		\
		dcmotor##motID.VolVars.lastIncTime = timeISR();				\
		dcmotor##motID.VolVars.IncDeltaT += timeISR();				\
		dcmotor##motID.Incr.incB = digitalRead(MOT##motID##_B); 	\
		if(dcmotor##motID.Incr.incA ^ !dcmotor##motID.Incr.incB) {	\
			dcmotor##motID.VolVars.Position++;						\
			dcmotor##motID.VolVars.direction = 1;					\
			if(dcmotor##motID.PosRamp.length && dcmotor##motID.VolVars.Position >= dcmotor##motID.PosRamp.length) \
				dcmotor##motID.VolVars.Position -= dcmotor##motID.PosRamp.length; \
		}															\
		else  {														\
			dcmotor##motID.VolVars.Position--;						\
			dcmotor##motID.VolVars.direction = 0;					\
			if(dcmotor##motID.PosRamp.length && dcmotor##motID.VolVars.Position < 0) \
				dcmotor##motID.VolVars.Position += dcmotor##motID.PosRamp.length; \
		}															\
	}																\
 } while(0)	
#define DCMOTOR_CAPTURE_SERVICE(motID) CALL_FUN(DCMOTOR_CAPTURE_SERVICE_,motID)

#define DCMOTOR_CAPTURE_SERVICE_SINGLE_(motID) do{ 					\
	dcmotor##motID.Incr.incA = digitalRead(MOT##motID##_A); 		\
	if(digitalRead(MOT##motID##_END) == MOT##motID##_ENDLEVEL) { 	\
		dcmotor##motID.VolVars.Position = 0; 						\
		dcmotor##motID.VolVars.homed = 1;							\
		dcmotor##motID.VolVars.end = 1;								\
	}	else dcmotor##motID.VolVars.end = 0;						\
	if(dcmotor##motID.Incr.incA != dcmotor##motID.Incr.lastA) { 	\
		dcmotor##motID.Incr.lastA = dcmotor##motID.Incr.incA;		\
		dcmotor##motID.VolVars.IncDeltaT -= dcmotor##motID.VolVars.lastIncTime;	\
		dcmotor##motID.VolVars.lastIncTime = timeISR();				\
		dcmotor##motID.VolVars.IncDeltaT += timeISR();				\
		dcmotor##motID.Incr.incB = digitalRead(MOT##motID##_B); 	\
		if(dcmotor##motID.VolVars.direction) dcmotor##motID.VolVars.Position++;	\
		else  dcmotor##motID.VolVars.Position--;					\
	}																\
 } while(0)	
#define DCMOTOR_CAPTURE_SERVICE_SINGLE(motID) CALL_FUN(DCMOTOR_CAPTURE_SERVICE_SINGLE_,motID)


#define DCMOTOR_INIT_(motID) do{\
	digitalClear(M##motID##1);\
	digitalClear(M##motID##2);\
	digitalSet(M##motID##EN);\
	digitalSet(M##motID##EN2);\
	pinModeDigitalOut(M##motID##1);\
	pinModeDigitalOut(M##motID##2);\
	pinModeDigitalOut(M##motID##EN);\
	pinModeDigitalOut(M##motID##EN2);\
    \
    pinModeDigitalIn(MOT##motID##_END);\
	pinModeDigitalIn(MOT##motID##_A);\
	pinModeDigitalIn(MOT##motID##_B);\
	INIT_PWM(MOT##motID##_PWM);\
	SET_PWM(MOT##motID##_PWM, 0);\
	MOT##motID##_CONFIG();\
	\
	dcmotor##motID.Setting.Mode = 0;\
	dcmotor##motID.Setting.PosWindow = 2;\
	dcmotor##motID.Setting.PwmMin = 10;\
	dcmotor##motID.Setting.StallTime = 16;\
	dcmotor##motID.Setting.PosErrorGain = 7;\
	dcmotor##motID.Setting.reversed = 0;\
	dcmotor##motID.Setting.onlyPositive = 1;\
	\
	rampInit(&dcmotor##motID.PosRamp);\
	pidInit(&dcmotor##motID.SpeedPID);\
	pidInit(&dcmotor##motID.PosPID);\
	\
	dcmotor##motID.VolVars.Position = 0;\
	dcmotor##motID.VolVars.homed = 0;\
	dcmotor##motID.VolVars.end = 0;\
	dcmotor##motID.Vars.PWMConsign = dcmotor##motID.Vars.SpeedConsign = 0;\
	dcmotor##motID.Vars.lastPosition = 0;\
 } while(0)	
#define dcmotorInit(motID) CALL_FUN(DCMOTOR_INIT_,motID)

#define DCMOTOR_LOAD_dcmotorVolVars_(motID) do{ \
	__critical {\
		dcmotorVolVars.lastIncTime = dcmotor##motID.VolVars.lastIncTime;\
		dcmotorVolVars.IncDeltaT = dcmotor##motID.VolVars.IncDeltaT;\
		dcmotorVolVars.Position = dcmotor##motID.VolVars.Position;\
		dcmotorVolVars.flags = dcmotor##motID.VolVars.flags;\
		dcmotor##motID.VolVars.IncDeltaT = 0;\
	}\
 } while(0)

void dcmotorCompute(t_dcmotor *mot);

#define DCMOTOR_FORMATPWM(motID) do{ \
	dcmotor_v = dcmotor##motID.Vars.PWMConsign; \
	if(dcmotor_v > 1023) dcmotor_v = 1023;	\
	if(dcmotor_v < -1023) dcmotor_v = -1023;	\
	if((dcmotor_v > 0) && (dcmotor_v < dcmotor##motID.Setting.PwmMin)) dcmotor_v = 0; \
	if((dcmotor_v < 0) && (dcmotor_v > -dcmotor##motID.Setting.PwmMin)) dcmotor_v = 0; \
	if(dcmotor##motID.Setting.onlyPositive && dcmotor##motID.VolVars.end && (dcmotor_v < 0)) dcmotor_v = 0;\
	/*dcmotor_v  = (dcmotor##motID.Setting.reversed ? -dcmotor_v : dcmotor_v);*/\
} while(0)

#define DCMOTOR_UPDATE_ASYM_(motID) do{ \
	DCMOTOR_FORMATPWM(motID);\
	dcmotor_vabs = dcmotor_v < 0 ? 1023 + dcmotor_v : dcmotor_v; \
	SET_PWM(MOT##motID##_PWM, dcmotor_vabs); \
	if(dcmotor_v < 0) { digitalSet(M##motID##2);}\
	else { digitalClear(M##motID##2);}\
 } while(0)
#define DCMOTOR_UPDATE_ASYM(motID) CALL_FUN(DCMOTOR_UPDATE_ASYM_,motID)

#define DCMOTOR_UPDATE_SYM_(motID) do{ \
	DCMOTOR_FORMATPWM(motID);\
	dcmotor_vabs = dcmotor_v < 0 ? -dcmotor_v : dcmotor_v; \
	SET_PWM(MOT##motID##_PWM, dcmotor_vabs); \
	if(dcmotor_v < 0) { digitalClear(M##motID##1);digitalSet(M##motID##2);}\
	else { digitalClear(M##motID##2); digitalSet(M##motID##1);}\
 } while(0)
#define DCMOTOR_UPDATE_SYM(motID) CALL_FUN(DCMOTOR_UPDATE_SYM_,motID)

#define DCMOTOR_COMPUTE(motID, mode) do{\
	DCMOTOR_LOAD_dcmotorVolVars_(motID);\
	dcmotorCompute(&dcmotor##motID);\
	DCMOTOR_UPDATE_##mode(motID);\
 } while(0)

#define DCMOTOR_SETDIR_(motID) do{ \
	if(dcmotor_v < 0) dcmotor##motID.VolVars.direction = 0;\
	else dcmotor##motID.VolVars.direction = 1;\
 } while(0)
 
#define DCMOTOR_COMPUTE_SINGLE(motID, mode) do{\
	DCMOTOR_COMPUTE(motID, mode);\
	DCMOTOR_SETDIR_(motID);\
 } while(0)

void dcmotorInput(t_dcmotor *mot);
#define DCMOTOR_INPUT(motID) dcmotorInput(&dcmotor##motID)

void dcmotorDeclareEE(t_dcmotor *mot);
#define DCMOTOR_DECLARE_EE(motID) do{ dcmotorDeclareEE(&dcmotor##motID);} while(0)

#define DCMOTOR(motID) (dcmotor##motID)
#define DCMOTOR_GETPOS(motID) (DCMOTOR(motID).VolVars.Position)

#ifndef RAMP_TO_POS_POW
#define RAMP_TO_POS_POW RAMP_UINCPOW
#endif


#endif // _DCMOTOR_H_
