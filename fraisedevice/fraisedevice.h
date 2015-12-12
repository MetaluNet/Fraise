/*********************************************************************
 *
 *                Fraise device firmware v2.1
 *
 *********************************************************************
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) Antoine Rousseau   2009 - 2015
 ********************************************************************/

#ifndef FRAISEDEV_H
#define FRAISEDEV_H
#include <core.h>

//-------------- SYSTEM -------------

void fraiseInit(void);
void fraiseService(void); // to be called by the main loop.
void fraiseSetID(unsigned char id); // normally you don't have to use this.
void fraiseISR(); //to be called by low priority interrupt.
void fraiseSetInterruptEnable(char enable); //fraiseInit automatically enables interrupt.
char fraiseGetInterruptEnable(void);

//-------------- OUT ----------------
/* fraiseSend :
	send buf,len to Fraise TX ring buffer.
	return 0 if success,-1 if overload, -2 if format error.
	First character must be 'B' for bytes or 'C' for characters.
*/
char fraiseSend(const unsigned char *buf,unsigned char len);


//-------------- IN ----------------
// Inputs prototypes ; user must define these four functions :
void fraiseReceiveCharBroadcast(); 	// string message to every device 
void fraiseReceiveBroadcast(); 		// raw message to every device 
void fraiseReceiveChar();		// string message
void fraiseReceive();			// raw message

// Input routines
unsigned char fraiseGetChar(); 	// get next char from receive buffer
unsigned char fraisePeekChar(); // see next char (but keep it in the buffer)
unsigned char fraiseGetIndex(); // get read index in RXbuffer
unsigned char fraiseGetAt(unsigned char i); // get RXbuffer content at a given place
unsigned char fraiseGetLen(); 	// total length of current receive packet.

void fraiseSendCopy(); // copy the RX buffer to TX buffer, in char mode, from first RX byte to the one before current index (don't add last fraiseGetChar). Used to return queried parameter setting.


// Input macros (parameters set/get)
#define PARAM_CHAR(n,p) case n: p = fraiseGetChar();
#define PARAM_INT(n,p) case n: p = fraiseGetChar() << 8; p += fraiseGetChar();
#define PARAM_LONG(n,p) case n: p = ((unsigned long)( \
	(((unsigned int)fraiseGetChar()) << 8) + fraiseGetChar()) ) << 16 | \
	(((unsigned int)fraiseGetChar()) << 8) + fraiseGetChar();

// Return the value of a parameter :
#define GETPARAM(n, p, i) case n: i = p; break

#endif //FRAISEDEV_H

