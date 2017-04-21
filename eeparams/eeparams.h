/*********************************************************************
 *
 *                Fraise eeprom stored parameters library
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
 * Copyright (c) Antoine Rousseau   nov 2011 - 2014
 ********************************************************************/

#ifndef EEPARAMS_H
#define EEPARAMS_H
/** @file */

#include "core.h"
/** @defgroup eeparams EEPROM parameters module
  Save parameters values into EEPROM, and load them at setup.
  
 -------------
 example :
 ~~~~
 #include "fruit.h"
 
 int i;
 
 void setup() {
 	EEreadMain(); // at boot time restore the value of "i" from EEPROM
 }
 
 void loop() { fruitService(); }
 
 void fraiseReceive() // if we received a "raw bytes" message
 {
 	unsigned char c = fraiseGetChar(); // get the first byte of the message

	switch(c) {
		PARAM_INT(1,i); // if the first byte was 1 then set "i" 
		 EEwriteMain() ;// to the value of the next 16 bit integer and save EEPROM.
		 break; 	
	}
 }
 
 void EEdeclareMain() {
 	EEdeclareInt(&i); // declare "i" as an 16 bit integer into EEPROM storage.
 }
 ~~~~
 *  @{
 */

/** \name Read and write functions
@{ */
/** \brief Load all the parameters values from EEPROM ; you may call it at setup() */ 
void EEreadMain();
/** \brief Save all the parameters values to EEPROM */ 
void EEwriteMain();
/** @} */

/** \name Parameters declaration functions
@{ */

/** \brief User may define this function, with a list of char/int/long declarations. */
void EEdeclareMain();

// EEdeclareMain() is definded with a sequence of :
/** \brief Declare a 8 bit integer in EEdeclareMain(). 
 @param data Address of the parameter.*/
void EEdeclareChar(unsigned char *data);
/** \brief Declare a 16 bit integer in EEdeclareMain(). 
 @param data Address of the parameter.*/
void EEdeclareInt(unsigned int *data);
/** \brief Declare a 32 bit integer in EEdeclareMain(). 
 @param data Address of the parameter.*/
void EEdeclareLong(unsigned long *data);
/// @}

/// @}



#endif //EEPARAMS_H
