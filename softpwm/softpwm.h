/*********************************************************************
 *
 *                Software PWM library for Fraise pic18f  device
 *				
 *				
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
 * Antoine Rousseau  april 2017     Original.
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
#ifndef _SOFTPWM__H_
#define _SOFTPWM__H_
/** @file */

/** @defgroup softpwm Software PWM module
 *  Implements 8channels(max) 10bits 100Hz(max) software PWM.
  
 *  Example :
 *   main.c
 * \include softpwm/examples/ex1/main.c
 *   config.h
 * \include softpwm/examples/ex1/config.h
 *  @{
 */

#include <fruit.h>


/** \name Settings to put in config.h
 Define in config.h the wanted pins: 
	~~~~
	    #define SOFTPWM0 K1 (replace K1 by the pin you want for softpwm channel 0)
	    #define SOFTPWM1 K2 (replace K2 by the pin you want for softpwm channel 1)
	    (...)
	~~~~

 The following parameters can be overloaded:
*/
//@{
#ifndef SOFTPWM_TIMER
/** @brief Timer to be used by the softpwm module (only 1, 3 or 5 ; default: 5). */
#define  SOFTPWM_TIMER 5
#endif

#ifndef SOFTPWM_MAX_VALUE
#define SOFTPWM_MAX_VALUE 1023 /**< @brief default 1023.*/
#endif

#ifndef SOFTPWM_TICK_US 
#define SOFTPWM_TICK_US 5 /**< @brief default 5 ; PWM frequency will be 1 / (SOFTPWM_MAX_VALUE * SOFTPWM_TICK_US) */
#endif

#ifndef SOFTPWM_INTPRI
#define SOFTPWM_INTPRI 1 /**< @brief default 1 (high priority).*/
#endif

//@}

/** \name Initialization
*/
//@{
/** @brief Init the module in setup() */
void softpwmInit();
//@}

/** \name Main loop functions
*/
//@{
//@}


/** \name Utilities
*/
//@{
/// @brief Set the value of a channel.
void softpwmSet(unsigned char chan, unsigned int val);
/// @brief Get the current value of a channel.
unsigned int softpwmGet(unsigned char chan);

//@}

/** \name Interrupt routine
*/
//@{
void softpwmHighInterrupt(void); ///< @brief Module high interrupt routine, must be called by the **highInterrupts()** user defined function.
//@}
//@{
void softpwmLowInterrupt(void); ///< @brief Module low interrupt routine, must be called by the **lowInterrupts()** user defined function.
//@}

/** \name Receive function
*/
//@{
/** @brief Module receive function, to be called by the **fraiseReceive()** user defined function.
* 
    The first byte of the message represents the channel (0-7), the 2 next bytes are the 16 bit new pwm value.
*/
void softpwmReceive();
//@}


/** @} 
*/

#endif
