/*
** This file is part of fsusb_picdem
**
** fsusb_picdem is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version.
**
** fsusb_picdem is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with fsusb_picdem; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
** 02110-1301, USA
*/


#ifndef __RJLHEX_H__
#define __RJLHEX_H__



#include <stdio.h>



typedef struct _hex_record {
  unsigned char datlen;
  unsigned int addr;
  unsigned char type;
  unsigned char checksum;
  unsigned char data[0];
} hex_record;

typedef struct _hex_file {
  FILE *f;
  unsigned long addr;
} hex_file;



// Create a hex_file from an already-open FILE *
hex_file *hex_open(FILE *f);

// Read the next hex_record from f
hex_record *hex_read(hex_file *f);



#endif /* __RJLHEX_H__ */
