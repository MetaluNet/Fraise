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

 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) Antoine Rousseau   nov 2011 - 2014
 ********************************************************************/

#ifndef EEPARAMS_H
#define EEPARAMS_H


// User EEPROM routines (parameters save/load) :
#define EEUSER 28 // first free eeprom address

/*extern int eeaddress;

#define EE_RD 0
#define EE_WR 1
extern char eeoperation;

#define EE_READBYTE_NEXT() (ee_read_byte((char)(eeaddress++)))
#define EE_WRITEBYTE_NEXT(data) ee_write_byte((char)(eeaddress++),data)
void EEsetaddress(int address); // init eeaddress if address>=EEUSER;

char EELoadChar();
int EELoadInt();
long EELoadLong();

void EESaveChar(unsigned char data);
void EESaveInt(int data);
void  EESaveLong(long data);*/

//User defined main input entries :
void EEdeclareMain();

// EEdeclareMain() is definded with a sequence of :
void EEdeclareChar(unsigned char *data);
void EEdeclareInt(unsigned int *data);
void EEdeclareLong(unsigned long *data);


//Functions to store/load all params set :
void EEwriteMain();
void EEreadMain();


#endif //EEPARAMS_H
