/*********************************************************************
 *
 *                Switch library for Fraise pic18f  device
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
#ifndef _SWITCH__H_
#define _SWITCH__H_

void Switch_Init();

void Switch_Select(unsigned char channel, unsigned char *port, unsigned char bit); // assign a port/bit to a channel
//ATTENTION : bit is litteral (0-7)

void Switch_Deselect(unsigned char channel); // deselect a channel

void Switch_Service(void); // call often

char Switch_Send(void); // call at the maximum rate you want to report switches
						// return number of channels sent

void Switch_Input(/*unsigned char fraddress*/);

char Switch_Get(unsigned char chan);

#define SWITCH_SELECT_(num,connport,connbit) \
	do {  \
	Switch_Select(num,&PORT##connport,connbit); } while(0)

//TRIS##connport##bits.TRIS##connport##connbit = 1; /*configure Kconn pin as an input*/	

// use next macro like this : SERVO_SET_PORT(0,K2); to assign Switch_0 to K2 connector.
#define SWITCH_SELECT(num,conn) CALL_FUN3(SWITCH_SELECT_,num,KPORT(conn),KBIT(conn))

#endif //
