/*********************************************************************
 *
 *                Fraise master firmware v2.1
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
 * Copyright (c) Antoine Rousseau   2009-2011
 ********************************************************************/

#ifndef FRAISEMASTER_H
#define FRAISEMASTER_H

void FraiseInit(void);

void FraiseService(void); // to be called by main loop
void FraiseISR(void); // interrupt service routine (high priority)
void FraiseSOF(void); // called on usb start of frame

void FrGetLineFromUsb(void);

#define LINE_FROM_USB_MAXLEN 64
extern unsigned char LineFromUsb[LINE_FROM_USB_MAXLEN];
extern unsigned char LineFromUsbLen;
extern unsigned char FrGotLineFromUsb;

#endif //FRAISEMASTER_H
