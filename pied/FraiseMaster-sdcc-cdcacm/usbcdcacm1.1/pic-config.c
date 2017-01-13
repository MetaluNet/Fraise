/*
File: pic-config.h

Copyright (c) 2009,2013 Kustaa Nyholm / SpareTimeLabs

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
Version 1.1     Compatible with SDCC 3.x

*/


__code char __at 0x300000 CONFIG1L = 0x20; // USBDIV=1, CPUDIV=00, PLLDIV = 000

__code char __at 0x300001 CONFIG1H = 0x0E; // IESO=0, FCMEN=0, FOSC = 1110

__code char __at 0x300002 CONFIG2L = 0x20; // Brown out off, PWRT On

__code char __at 0x300003 CONFIG2H = 0x00; // WDT off

__code char __at 0x300004 CONFIG3L = 0xff; // Unused configuration bits

__code char __at 0x300005 CONFIG3H = 0x81; // Yes MCLR, PORTB digital, CCP2 - RC1

__code char __at 0x300006 CONFIG4L = 0x80; // ICD off, ext off, LVP off, stk ovr off

__code char __at 0x300007 CONFIG4H = 0xff; // Unused configuration bits

__code char __at 0x300008 CONFIG5L = 0xff; // No __code read protection

__code char __at 0x300009 CONFIG5H = 0xff; // No data/boot read protection

__code char __at 0x30000A CONFIG6L = 0xff; // No __code write protection

__code char __at 0x30000B CONFIG6H = 0xff; // No data/boot/table protection

__code char __at 0x30000C CONFIG7L = 0xff; // No table read protection

__code char __at 0x30000D CONFIG7H = 0xff; // No boot table protection
