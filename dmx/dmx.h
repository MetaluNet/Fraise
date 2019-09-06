/*********************************************************************
 *
 *                DMX library for Fraise pic18f device
 *
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
 * Antoine Rousseau  may 2012     Original.
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
#ifndef _DMX_H_
#define _DMX_H_

/** @file */

/** @defgroup dmx DMX master module
 *  
  
 *  Example :
  
 *  **config.h** :
 * \include dmx/examples/example1/config.h
 *  **main.c** :
 * \include dmx/examples/example1/main.c
 *  @{
 */
#include <fruit.h>

/** \name Settings to put in config.h
	You must define the serial port :
	~~~~
	    #define DMX_UART_NUM [1/2]
	    #define DMX_UART_PIN [pin]
	~~~~
*/

//@{
#ifndef DMX_NBCHAN 
#define DMX_NBCHAN 128 /**< @brief DMX_NBCHAN default is 128, maybe overloaded up to 256 (TODO:512 but memory issue at compile time when tested). */
#endif

//@}

/** \name Initialization
*/
//@{
/** @brief Init the module in **setup()** */
void DMXInit();
//@}

/** \name Main loop functions
*/
//@{
void DMXService(); ///< @brief Module service routine, to be called by the main **loop()**.
//@}

/** \name Utilities
*/
//@{
/** @brief Set value of a DMX channel. 
    @param channel DMX channel (starting from 1)
    @param value New value for this channel (0-255)
*/
void DMXSet(unsigned int channel, unsigned char value);
//@}

extern unsigned int DMXframeCount;
/** @} 
*/

#endif // _DMX_H_

