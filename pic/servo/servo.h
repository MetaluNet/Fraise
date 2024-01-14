/*********************************************************************
 *
 *                Servo library for Fraise pic18f  device
 *				Uses TIMER5 !
 *				
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
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

/** @file */

/** @defgroup servo Servo module
 *  Automates the use of maximum 8 RC servomotors.
    Servo module uses timer 5.
  
 *  Example :
 * \include servo/examples/example1/main.c
 *  @{
 */
#include <fruit.h>

/** \name Initialization
*/
//@{
/** @brief Init the module in **setup()** */
void servoInit(void);

/** @brief Select a pin for a servo channel. 
    @param num Servomotor channel (0 to 7)
    @param conn Symbol of the pin (example : K1 for connector 1)	
*/
#define servoSelect(num,conn) do { \
		digitalClear(conn);\
		pinModeDigitalOut(conn);\
		CALL_FUN3(SERVO_SELECT_,num,KPORT(conn),KBIT(conn));\
	} while(0)
//@}

	
/** \name Main loop functions
*/
//@{
void servoService(void); ///< @brief Module service routine, to be called by the main **loop()**.
//@}

/** \name Utilities
*/
//@{
/** @brief Set position of a servomotor. 
    @param num Servomotor channel (0 to 7)
    @param val New position of this servo, in 8/FOSC steps ; e.g for Versa1, FOSC=64MHz, so servo steps are 1/8 us : 8000 corresponds to 1 ms position.  
*/
void servoSet(unsigned char num,unsigned int val); 
//@}


/** \name Interrupt routine
*/
//@{
void servoHighInterrupt(void); ///< @brief Module interrupt routine, must be called by the **highInterrupts()** user defined function.
//@}

/** \name Receive function
*/
//@{
/** @brief Module receive function, to be called by the **fraiseReceive()** user defined function.
* 
    The first byte of the message represents the channel (0-7), the 2 next bytes are the 16 bit new position value.
    If the first byte equals to 254, then the message is for reading a channel position ; the next byte then is the actual channel, and the module sends to the master the current position of this channel.
*/
void servoReceive();
//@}

void servoSetPort(unsigned char num,unsigned char *port,unsigned char mask);

#define SERVO_SELECT_(num,connport,connbit) servoSetPort(num,&LAT##connport,1<<connbit)
	
/** @} 
*/

#endif //
