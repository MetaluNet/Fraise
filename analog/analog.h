/*********************************************************************
 *
 *                Analog library for Fraise pic18f  device
 *				
 *				
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

#include <fruit.h>
/*#include <core.h>
#include <fraisedevice.h>
#include <eeparams.h>*/

#ifndef ANALOG_MAX_CHANNELS
#define ANALOG_MAX_CHANNELS 16
#endif

#ifndef ANALOG_FILTER
#define ANALOG_FILTER 3
#endif

#ifndef ANALOG_THRESHOLD
#define ANALOG_THRESHOLD 7
#endif

#ifndef ANALOG_MINMAX_MARGIN
#define ANALOG_MINMAX_MARGIN 100
#endif

#ifndef ANALOG_SCALED_MAX
#define ANALOG_SCALED_MAX 16383
#endif

#define AMODE_SCALE 1
#define AMODE_NOSCALE 0
#define AMODE_NUM 2
#define AMODE_CHAR 0
#define AMODE_CROSS 4
#define AMODE_NOCROSS 0

void analogInit();

void analogSelectAdc(unsigned char chan,unsigned char hwchan); // attach a hardware channel to a channel
#define analogSelect(num,conn) do { pinModeAnalogIn(conn); CALL_FUN2(analogSelectAdc,num,KAN(conn)); } while(0)

void analogDeselect(unsigned char chan); // deselect a channel

void analogService(void); // call often

char analogSend(unsigned char mode); 	// call at the maximum rate you want to report analog ; 
					// return number of channels sent (max 4)

//void analogInput();

void analogSet(unsigned char chan, int val);
int analogGet(unsigned char chan);

int analogGetDistance(unsigned char chan);

void analogScaling(unsigned char scaling); // when scaling, min and max are updated each sample

void analogDeclareEE();

void analogPrintMin();
void analogPrintMax();



#endif //
