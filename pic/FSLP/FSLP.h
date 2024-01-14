/*********************************************************************
 *
 *                Force-sensing linear potentiometers (FSLPs) from Interlink Electronics
 *                for Fraise pic18f  device
 *
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
 * Antoine Rousseau  feb 2021     Original.
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
#ifndef _FSLP__H_
#define _FSLP__H_
/** @file */

/** @defgroup FSLP Force-sensing linear potentiometers module
 *  
  
 *  Example :
 * \include analog/examples/example1/main.c
 *  @{
 */

#include <fruit.h>

/*
       ___________________________________
  SL---|                                 |
  D2---| . . . . . . . . . . . . . . . . |
  D1---|_________________________________|
  
see: https://www.pololu.com/product/2730

*/

/** \name Settings to put in config.h
 These parameters can be set in the config.h of your firmware.
*/
//@{
/// @brief FSLP_D2 and FSLP_SL must be analog input capable. FSLP_R0 must be connected to FSLP_SL through 4.7k
#if !(defined(FSLP_D1) && defined(FSLP_D2) && defined(FSLP_SL) && defined(FSLP_R0))
#error you must define FSLP_D1, FSLP_D2, FSLP_SL and FSLP_R0! FSLP_D2 and FSLP_SL must be analog input capable. R0 must be connected to SL through 4.7k
#endif

#ifndef FSLP_D2CHAN
#define FSLP_D2CHAN 0
#endif

#ifndef FSLP_SLCHAN
#define FSLP_SLCHAN 1
#endif

#ifndef FSLP_SLCHAN2
#define FSLP_SLCHAN2 2
#endif

#ifndef ANALOG_CHANNEL_PRECONF
#error you must define ANALOG_CHANNEL_PRECONF! 
#error Add in your config.h: #define ANALOG_CHANNEL_PRECONF FSLP_analog_preconf
#endif

//@}

void FSLP_analog_preconf(unsigned char chan);

/** \name Initialization
*/
//@{
/** @brief Init the module in setup() */
void FSLP_Init();

//@}

/** @} 
*/

#endif
