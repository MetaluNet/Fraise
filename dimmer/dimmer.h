/*********************************************************************
 *
 *                8 channels AC dimmer library for Fraise pic18f device
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
 *
 *   main.c
 * \include dimmer/examples/example1/main.c
 *   config.h
 * \include dimmer/examples/example1/config.h
 *  @{
 */
#include <fruit.h>
/** \name Settings to put in config.h
 You must define the pin used to trig the interrupt (sensing the AC zero-crossing):
	~~~~
	    #define DIMMER_INTPIN K9 (replace K9 by the pin used by your dimmer)
	~~~~

 The following parameters can be overloaded:
*/
//@{

#ifndef DIMMER_INTPIN
#error you must define DIMMER_INTPIN before calling dimmer.h
#endif

#ifndef DIMMER_INTEDGE
/** @brief Edge of the interrupt (0:falling edge ; 1:rising edge) ; default: 1(rising edge). */
#define  DIMMER_INTEDGE 1
#endif

#ifndef DIMMER_TIMER
/** @brief Timer to be used by the dimmer module (only 1, 3 or 5 ; default: 5). */
#define  DIMMER_TIMER 5
#endif

#ifndef DIMMER_K0 
#define DIMMER_K0 KZ2 /**< @brief output pin for channel 0.*/
#endif
#ifndef DIMMER_K1 
#define DIMMER_K1 KZ2 /**< @brief output pin for channel 1.*/
#endif
#ifndef DIMMER_K2 
#define DIMMER_K2 KZ2 /**< @brief output pin for channel 2.*/
#endif
#ifndef DIMMER_K3 
#define DIMMER_K3 KZ2 /**< @brief output pin for channel 3.*/
#endif
#ifndef DIMMER_K4 
#define DIMMER_K4 KZ2 /**< @brief output pin for channel 4.*/
#endif
#ifndef DIMMER_K5 
#define DIMMER_K5 KZ2 /**< @brief output pin for channel 5.*/
#endif
#ifndef DIMMER_K6 
#define DIMMER_K6 KZ2 /**< @brief output pin for channel 6.*/
#endif
#ifndef DIMMER_K7 
#define DIMMER_K7 KZ2 /**< @brief output pin for channel 7.*/
#endif

#ifndef DIMMER_TMIN
#define DIMMER_TMIN 9000UL
#endif

#ifndef DIMMER_INTPRI
#define DIMMER_INTPRI 1
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
/** @brief Set the value of a dimmer channel.
    @param num Dimmer channel (0 to 7)
    @param val New 16 bit value for this channel, between 0 and 65535 
*/
void dimmerSet(unsigned char num,unsigned int val); 
//@}


/** \name Interrupt routine
*/
//@{
void dimmerHighInterrupt(void); ///< @brief Module high interrupt routine, must be called by the **highInterrupts()** user defined function.
//@}
//@{
void dimmerLowInterrupt(void); ///< @brief Module low interrupt routine, must be called by the **lowInterrupts()** user defined function.
//@}

/** \name Receive function
*/
//@{
/** @brief Module receive function, to be called by the **fraiseReceive()** user defined function.
* 
    The first byte of the message represents the channel (0-7), the 2 next bytes are the 16 bit new position value.
    If the first byte is 8 then the next char sets the AC frequency : 0=50Hz, 1=60Hz
*/
void dimmerReceive();
//@}

void dimmerPrintDebug();

/** @} 
*/

#endif // DIMMER
