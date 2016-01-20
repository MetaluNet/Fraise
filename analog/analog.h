/*********************************************************************
 *
 *                Analog library for Fraise pic18f  device
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
#ifndef _ANALOG__H_
#define _ANALOG__H_

/** @defgroup analog Analog module
 *  Automates the use of analog input pins.
 *  @{
 */

#include <fruit.h>


/** \name Settings to put in config.h
 These parameters can be overloaded in the config.h of your firmware.
*/
//@{

#ifndef ANALOG_MAX_CHANNELS 
#define ANALOG_MAX_CHANNELS 16 /**< @brief default 16.*/
#endif

#ifndef ANALOG_FILTER 
#define ANALOG_FILTER 3 /**< @brief default 3 ; analog values are filtered and multiplied by 1<<ANALOG_FILTER. */
#endif

#ifndef ANALOG_THRESHOLD
/** @brief default 7 : don't send an analog channel until it differs from last sent value by more than ANALOG_THRESHOLD. */
#define ANALOG_THRESHOLD 7
#endif

#ifndef ANALOG_MINMAX_MARGIN
/** @brief default 100 : increase the minimum measured value by this amount (and decrease max value) for scaling output. */
#define ANALOG_MINMAX_MARGIN 100 
#endif

#ifndef ANALOG_SCALED_MAX
/** @brief default 16383 : maximum scaled output value. */
#define ANALOG_SCALED_MAX 16383
#endif

//@}

/** \name Output mode switchs
 The different mode switchs can be OR-ed together and passed to analogSetMode().
 Default is AMODE_NUM.
*/
//@{
/** @brief Map each channel to a normalized scale (see analogScaling() ). */
#define AMODE_SCALE 1 
/** @brief Send values in a raw (numerical) message, parsed by analog/parse.pd patch. Otherwise send text messages : "A channel value".*/
#define AMODE_NUM 2
/** @brief Crossing mode.  
If channel has been set (see analogSet() ), wait the measurement value has crossed the set value before to continue to send value updates.*/
#define AMODE_CROSS 4
//@}

/** @brief Init the module in setup() */
void analogInit();

void analogSelectAdc(unsigned char chan,unsigned char hwchan); // attach a hardware channel to an analog channel

/** @brief Select a pin for an analog channel. 
	Assign the pin to the analog channel.
    @param num Number of the channel (first channel = 0)
    @param conn Symbol of the pin (example : K1 for connector 1)	
*/
#define analogSelect(num,conn) do { pinModeAnalogIn(conn); CALL_FUN2(analogSelectAdc,num,KAN(conn)); } while(0)

void analogDeselect(unsigned char chan); ///< @brief Deselect a channel.

void analogService(void); ///< @brief Module service routine, to be called by the main loop().

/** @brief Send analog values that changed. *//**
Call at the maximum rate you want to report analog.   
The way values are sent depends on the Output mode switchs. See analogSetMode().
@return number of channels sent (max 4) */
char analogSend(void); 	

/// @brief Configure the way analog values are sent (use Output mode switchs).
void analogSetMode(unsigned char mode); 

/// @brief Set the value of a channel (to be used in conjunction with AMODE_CROSS).
void analogSet(unsigned char chan, int val);
/// @brief Get the last measured value of a channel.
int analogGet(unsigned char chan);

/// @brief Get the distance between the last measured value of a channel and its internal value (set by analogSet() )
int analogGetDistance(unsigned char chan);

/// @brief Start or stop the scaling calibration
/// @param scaling 1:start 0:stop
/// First use analogScaling(1) to start calibration, which will measure the minimum and maximum values for each channel ; then stop calibration with analogScaling(0). 
void analogScaling(unsigned char scaling); // when scaling, min and max are updated each sample

/// @brief EEPROM declaration for this module
/// Call this function in your EEdeclareMain() if you want to save analog scaling calibration.
void analogDeclareEE();

/** @} 
*/

#endif
