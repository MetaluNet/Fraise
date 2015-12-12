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

#ifndef __BOOTLOAD_H__
#define __BOOTLOAD_H__



/*
 * Command packets:
 *
 * 0x00: command
 * 0x01: data length (usually; different action for some commands!)
 * 0x02: address bits 7..0
 * 0x03: address bits 15..8
 * 0x04: address bits 23..16 (upper bits always zero)
 * 0x05: data[0]
 * 0x06: data[1]
 * 0x07: data[2]
 * 0x??: ...
 * 0x3f: data[BL_DATA_LEN-1]
 */



typedef unsigned char byte;



#define BL_PACKET_LEN 64
#define BL_HEADER_LEN  5 // command, len, low, high, upper
#define BL_DATA_LEN   (BL_PACKET_LEN - BL_HEADER_LEN)



enum {
  READ_VERSION    = 0x00, // Works
  READ_FLASH      = 0x01, // Works
  WRITE_FLASH     = 0x02, // Works
  ERASE_FLASH     = 0x03, // Works
  READ_EEDATA     = 0x04, // NOT IMPLEMENTED
  WRITE_EEDATA    = 0x05, // NOT IMPLEMENTED
  READ_CONFIG     = 0x06, // NOT IMPLEMENTED
                          // (but in current firmware READ_FLASH works
  WRITE_CONFIG    = 0x07, // NOT TESTED
  UPDATE_LED      = 0x32, // NOT IMPLEMENTED
  RESET           = 0xFF  // NOT IMPLEMENTED
};



typedef union _bl_packet {
  byte _byte[64];
  struct {
    byte command;
    byte len;
    struct {
      byte low;
      byte high;
      byte upper;
    } address;
    byte data[BL_DATA_LEN];
  };
} bl_packet;

#endif /* __BOOTLOAD_H__ */
