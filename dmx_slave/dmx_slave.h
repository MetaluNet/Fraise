/*********************************************************************
 *
 *                DMX slave library for Fraise pic18f device
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
#ifndef _DMX_SLAVE_H_
#define _DMX_SLAVE_H_

void DMXSlaveInit();
void DMXSlaveISR(); // to be declared in low ISR
unsigned char DMXSlaveGet(unsigned int channel);
unsigned int DMXSlaveGetMaxReceivedChannel(void); // calling this function resets the MaxReceivedChannel count.
//  Application must define following constant in config.h :
//#define DMX_SLAVE_UART_PORT 	{1 or 2}
void DMXSlaveSet(unsigned int channel, unsigned char value);

#ifndef DMX_SLAVE_NBCHAN
#define DMX_SLAVE_NBCHAN 128
#endif

#endif // _DMX_SLAVE_H_

