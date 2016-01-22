/*********************************************************************
 *
 *                Switch library for Fraise pic18f  device
 *				
 *				
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
 * Antoine Rousseau  march 2013     Original.
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
#ifndef _SWITCH__H_
#define _SWITCH__H_
/** @file */

#include <fruit.h>
/** @defgroup switch Switch module
 *  Automates the use of digital input pins.
  
 *  Example :
 * \include switch/examples/example1/main.c
 * 
 *  @{
 */
void switchSelectHW(unsigned char channel, unsigned char *port, unsigned char bit); // assign a port/bit to a channel ; bit is decimal (0-7)
#define SWITCH_SELECT_(num,connport,connbit) \
	do { switchSelectHW(num,&PORT##connport,connbit); } while(0)

/// @name Initialization functions
/// @{
/** @brief Call it once in setup() */
void switchInit();
// use next macro like this : SERVO_SET_PORT(0,K2); to assign switch_0 to K2 connector.
/** @brief Select a pin for a switch channel. 
	Assign the pin to the switch channel.
    @param num Number of the channel (first channel = 0).
    @param conn Symbol of the pin (example : K1 for connector 1).	
*/
#define switchSelect(num,conn) do { \
	pinModeDigitalIn(conn); \
	CALL_FUN3(SWITCH_SELECT_,num,KPORT(conn),KBIT(conn)); } while(0)
///@}


/// @name Loop functions
/// @{
/** @brief Call in loop(). */
void switchService(void); 
/// @brief Call at the maximum rate you want to report switches.
/// @return Number of channels sent.
char switchSend(void);
///@}

/// @name Utilities
/// @{
/** @brief Deselect a channel. */
void switchDeselect(unsigned char channel); 
/// @brief Get the state of a channel.
/// @param chan Channel to read.
/// @return State of the channel.
char switchGet(unsigned char chan);
///@}


/// @}
#endif //
