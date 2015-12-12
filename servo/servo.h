/*********************************************************************
 *
 *                Servo library for Fraise pic18f  device
 *				Uses TIMER5 !
 *				
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  jan 2013     Original.
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
#ifndef _Servo__H_
#define _Servo__H_

void Servo_Init(void);

void Servo_SetPort(unsigned char num,unsigned char *port,unsigned char mask);
void Servo_Set(unsigned char num,unsigned int val);

void Servo_Service(void);
void Servo_Rewind(void); //to be called every 20ms

void Servo_ISR(void);

void Servo_Input();//unsigned char fraddress);

#define SERVO_SET_PORT_(num,connport,connbit) do {\
	LAT##connport##bits.LAT##connport##connbit = 0; /*set out latch to 0*/ \
	TRIS##connport##bits.TRIS##connport##connbit = 0; /*configure Kconn pin as an output*/ \
	Servo_SetPort(num,&LAT##connport,1<<connbit); } while(0)
	
// use next macro like this : SERVO_SET_PORT(0,2); to assign Servo_0 to K2 connector.
#define SERVO_SET_PORT(num,conn) CALL_FUN3(SERVO_SET_PORT_,num,KPORT(conn),KBIT(conn))

#endif //
