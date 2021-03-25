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
#include "FSLP.h"
#include <analog.h>

void FSLP_analog_preconf(unsigned char chan)
{
	if(chan == FSLP_D2CHAN) {
		pinModeAnalogIn(FSLP_D2);
		pinModeDigitalOut(FSLP_R0);
		digitalClear(FSLP_R0);
	} else if (chan == FSLP_SLCHAN2) {
		pinModeAnalogIn(FSLP_R0);
		pinModeDigitalOut(FSLP_D2);
		digitalClear(FSLP_D2);
	}
}

void FSLP_Init()
{
	analogSelect(FSLP_D2CHAN,FSLP_D2);
	analogSelect(FSLP_SLCHAN,FSLP_SL);
	analogSelect(FSLP_SLCHAN2,FSLP_SL);
	pinModeDigitalOut(FSLP_D1);
	digitalSet(FSLP_D1);
}

