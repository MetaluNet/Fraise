/*********************************************************************
 *
 *                Fraise eeprom stored user parameters library
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
 * Copyright (c) Antoine Rousseau   2009-2013   
 ********************************************************************/
 
#include "eeparams.h"

#define EEUSER 28 // first free eeprom address, after space reserved for ID, NAME and PREFIX.

int eeaddress;
char eeoperation;

#define EE_RD 0
#define EE_WR 1

#define EE_READBYTE_NEXT() (eeReadByte((char)(eeaddress++)))
#define EE_WRITEBYTE_NEXT(data) eeWriteByte((char)(eeaddress++),data)

// ------- Loads :

char EELoadChar()
{
	return EE_READBYTE_NEXT();
}

int EELoadInt()
{
	return (EE_READBYTE_NEXT()<<8)|EE_READBYTE_NEXT();
}

long EELoadLong() //untested
{
	return ((unsigned long)EE_READBYTE_NEXT()<<24)|((unsigned long)EE_READBYTE_NEXT()<<16)|(EE_READBYTE_NEXT()<<8)|EE_READBYTE_NEXT();
}


// ------- Saves :

void EESaveChar(unsigned char data)
{
	EE_WRITEBYTE_NEXT(data);
}

void EESaveInt(int data)
{
	EE_WRITEBYTE_NEXT(data>>8);
	EE_WRITEBYTE_NEXT(data&255);
}

void EESaveLong(long data) //untested
{
	EE_WRITEBYTE_NEXT(data>>24);
	EE_WRITEBYTE_NEXT(data>>16);
	EE_WRITEBYTE_NEXT(data>>8);
	EE_WRITEBYTE_NEXT(data);
}


// ------- Declares :

void EEdeclareChar(unsigned char *data)
{
	if(eeoperation==EE_RD) *data=EELoadChar();
	else if(eeoperation==EE_WR) EESaveChar(*data);
}

void EEdeclareInt(unsigned int *data)
{
	if(eeoperation==EE_RD) *data=EELoadInt();
	else if(eeoperation==EE_WR) EESaveInt(*data);
}

void EEdeclareLong(unsigned long *data)
{
	if(eeoperation==EE_RD) *data=EELoadLong();
	else if(eeoperation==EE_WR) EESaveLong(*data);
}


// ------- Read/Write :

void EEwriteMain()
{
	eeoperation=EE_WR;
	eeaddress=EEUSER;
#ifdef UD_EE
	EEdeclareMain();
#endif
}

void EEreadMain()
{
	eeoperation=EE_RD;
	eeaddress=EEUSER;
#ifdef UD_EE
	EEdeclareMain();
#endif
}


