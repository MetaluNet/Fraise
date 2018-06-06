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

*********************************************************************
 * Copyright (c) Antoine Rousseau   2009 - 2015
 ********************************************************************/

#ifndef FRAISEDEV_H
#define FRAISEDEV_H

/** @file */

#include <core.h>

/** @defgroup fraisedevice Fraise device module
 *  Implements Fraise device protocol on the serial device.
 *  @{
 */


//-------------- SYSTEM -------------

/** @name Initialization */
/** @{ */
/** @brief Init fraise device stack. Normally automatically called by fruitInit().*/
/** Configure serial port as a fraise device. */
void fraiseInit(void);
/** @} */

/** @name Service */
/** @{ */
/** @brief Fraise device service routine. */
/** Check for input data from Fraise.
 This function calls input functions when the device receives a message from master.<br>
 Normal users shouldn't call fraiseService() directly, but use fruitService() instead. */
void fraiseService(void); // to be called by the main loop.
/** @} */

void fraiseSetID(unsigned char id); // normally you don't have to use this.
void fraiseISR(); //to be called by low priority interrupt.
void fraiseSetInterruptEnable(char enable); //fraiseInit automatically enables interrupt.
char fraiseGetInterruptEnable(void);

//-------------- OUT ----------------

/** @name Fraise output to master */
/** @{ */
/** @brief Put a message into the Fraise TX queue
    @param buf Address of the bytes buffer; the buffer must start with either 'C' (string message) or 'B' (raw bytes)
    @param len Number of bytes in the buffer 
    @return 0 : success 
    @return -1 : TX queue overload
    @return -2 : TX buffer format error
*/
char fraiseSend(const unsigned char *buf,unsigned char len);

/** @name Broadcast output */
/** @{ */
/** @brief Broadcast a message to every connected fruit; this fonction is blocking (it returns when the message is sent)
    @param buf Address of the bytes buffer; the buffer must start with either 'C' (string message) or 'B' (raw bytes)
    @param len Number of bytes in the buffer 
*/
void fraiseSendBroadcast(const unsigned char *buf, unsigned char len);

/** @} */


//-------------- IN ----------------
/** \name Input Functions prototypes
	User may define any of these four functions, that are called when the device receives a message. Each input function is for a specific type of message. These functions then parse the received message using one of the Input Routines or Input Macros.
*/
//@{
void fraiseReceiveCharBroadcast(); 	/**< @brief String message to every device. */
void fraiseReceiveBroadcast(); 		/**< @brief Raw message to every device. */
void fraiseReceiveChar();		/**< @brief String message to this device. */
void fraiseReceive();			/**< @brief Raw message to this device. */
//@}

/** \name Input routines
	Use these functions when parsing the current message in Input Functions.
*/
//@{
unsigned char fraiseGetChar(); 	///< Get next char from receive buffer.
unsigned char fraisePeekChar(); ///< See next char (but keep it in the buffer).
unsigned char fraiseGetIndex(); ///< Get read index in RXbuffer.
unsigned char fraiseGetAt(unsigned char i); ///< Get RXbuffer content at a given place.
unsigned char fraiseGetLen(); 	///< Get total length of current receive packet.
void fraiseSendCopy(); ///< Copy the RX buffer to TX buffer, in char mode, from first RX byte to the one before current index (don't add last fraiseGetChar). Used to return queried parameter setting.

#define fraiseGetInt() ((unsigned int)(fraiseGetChar()<<8) + (unsigned int)fraiseGetChar()) ///< @brief Get next 16 bit integer from receive buffer.
#define fraiseGetLong() (((unsigned long)( \
	(((unsigned int)fraiseGetChar()) << 8) + fraiseGetChar()) ) << 16 | \
	(((unsigned int)fraiseGetChar()) << 8) + fraiseGetChar()) ///< @brief Get next 32 bit long integer from receive buffer.

//@}
	
/** \name  Input macros (parameters set/get)
	To be used in a switch() block.
*/
//@{
/// in case **n**, assign next byte in the message to **p**
#define PARAM_CHAR(n,p) case n: p = fraiseGetChar();
/// in case **n**, assign next 16 bit integer in the message to **p**
#define PARAM_INT(n,p) case n: p = fraiseGetChar() << 8; p += fraiseGetChar();
/// in case **n**, assign next 32 bit integer in the message to **p**
#define PARAM_LONG(n,p) case n: p = ((unsigned long)( \
	(((unsigned int)fraiseGetChar()) << 8) + fraiseGetChar()) ) << 16 | \
	(((unsigned int)fraiseGetChar()) << 8) + fraiseGetChar();

/// in case **n**, set **i** to the value of parameter **p**
#define GETPARAM(n, p, i) case n: i = p; break
//@}

/** @} 
*/

#endif //FRAISEDEV_H

