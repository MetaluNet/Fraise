/*********************************************************************
 *
 *                8 channels AC dimmer library for Fraise pic18f device
 *				Uses TIMER3 !
 *				
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
 * Antoine Rousseau  apr 2016     Original.
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
#ifndef _DIMMER__H_
#define _DIMMER__H_

/** @file */

/** @defgroup dimmer Dimmer module
 *  8 channels AC dimmer 
  
 *  Example :
 * \include dimmer/examples/example1/main.c
 *  @{
 */
#include <fruit.h>
/** \name Settings to put in config.h
 These parameters can be overloaded in the config.h of your firmware.
*/
//@{

#ifndef DIMMER_TIMER
/** @brief Timer to be used by the dimmer module (default: 5). */
#define  DIMMER_TIMER 5
#endif

#ifndef DIMMER_INTPIN
/** @brief Pin used to trig the interrupt (you MUST define it in your config.h). */
#error you must define DIMMER_INTPIN before calling dimmer.h
#endif

#ifndef DIMMER_INTEDGE
/** @brief Edge of the interrupt (0:falling edge ; 1:rising edge) ; default: 0 (falling edge). */
#define  DIMMER_INTEDGE 0
#endif

#ifndef DIMMER_K0 
#define DIMMER_K0 KZ2 /**< @output pin for channel 0.*/
#endif
#ifndef DIMMER_K1 
#define DIMMER_K1 KZ2 /**< @output pin for channel 1.*/
#endif
#ifndef DIMMER_K2 
#define DIMMER_K2 KZ2 /**< @output pin for channel 2.*/
#endif
#ifndef DIMMER_K3 
#define DIMMER_K3 KZ2 /**< @output pin for channel 3.*/
#endif
#ifndef DIMMER_K4 
#define DIMMER_K4 KZ2 /**< @output pin for channel 4.*/
#endif
#ifndef DIMMER_K5 
#define DIMMER_K5 KZ2 /**< @output pin for channel 5.*/
#endif
#ifndef DIMMER_K6 
#define DIMMER_K6 KZ2 /**< @output pin for channel 6.*/
#endif
#ifndef DIMMER_K7 
#define DIMMER_K7 KZ2 /**< @output pin for channel 7.*/
#endif

//@}
	
/** \name Initialization
*/
//@{
/** @brief Init the module in **setup()** */
void dimmerInit(void);

//@}

/** \name Main loop functions
*/
//@{
void dimmerService(void); ///< @brief Module service routine, to be called by the main **loop()**.
//@}

/** \name Utilities
*/
//@{
/** @brief Set position of a dimmermotor. 
    @param num Dimmermotor channel (0 to 7)
    @param val New delay for this dimmer, in 4/FOSC steps ; e.g for Versa1, FOSC=64MHz, so dimmer steps are 1/4 us : 40000 corresponds to 10 ms position (fully off @50Hz), 33333 gives a 8.333ms (fully off @60Hz)
*/
void dimmerSet(unsigned char num,unsigned int val); 
//@}


/** \name Interrupt routine
*/
//@{
void dimmerHighInterrupt(void); ///< @brief Module interrupt routine, must be called by the **highInterrupts()** user defined function.
//@}

/** \name Receive function
*/
//@{
/** @brief Module receive function, to be called by the **fraiseReceive()** user defined function.
* 
    The first byte of the message represents the channel (0-7), the 2 next bytes are the 16 bit new position value.
    If the first byte equals to 254, then the message is for reading a channel position ; the next byte then is the actual channel, and the module sends to the master the current position of this channel.
*/
void dimmerReceive();
//@}


#endif // DIMMER
