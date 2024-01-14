/*********************************************************************
 *
 *     Load Cell Amplifier HX711 library for Fraise pic18f device.
 * thanks to https://github.com/bogde/HX711 arduino library.
 *
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
 * Antoine Rousseau  sept 2016     Original.
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
#ifndef _HX711_H_
#define _HX711_H_

/** @file */

/** @defgroup hx711 Load Cell Amplifier HX711 module
 *  
  
 *  Example :
  
 *  **config.h** :
 * \include hx711/examples/example1/config.h
 *  **main.c** :
 * \include hx711/examples/example1/main.c
 *  @{
 */
#include <fruit.h>

#ifndef HX711_SCK_PIN
#error you must define HX711_SCK_PIN before calling hx711.h
#endif
#ifndef HX711_DATA_PIN
#error you must define HX711_DATA_PIN before calling hx711.h
#endif

/** \name Settings to put in config.h
	You must define the serial port :
	~~~~
	    #define HX711_SCK_PIN [pin]
	    #define HX711_DATA_PIN [pin]
	~~~~
	
*/

// The following parameters can be overloaded:

//@{

//@}

/** \name Initialization
*/
//@{
/** @brief Init the module in **setup()** 
    @param gainA channel A gain selection : 0=128 1=64
*/
void hx711Init(unsigned char gainA);
//@}

/** \name Main loop functions
*/
//@{
void hx711Service(); ///< @brief Module service routine, to be called by the main **loop()**.
//@}


/** \name Utilities
*/
//@{
/** @brief Read last measure of a channel
    @param channel channel selection : 0=A 1=B
    @return current measure
*/
long hx711Read(unsigned char channel);
//@}

/** @} 
*/

#endif // _HX711_H_

