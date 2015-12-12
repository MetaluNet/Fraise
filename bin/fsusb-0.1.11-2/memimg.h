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

#ifndef __MEMIMG_H__
#define __MEMIMG_H__

#include "rjlhex.h"
/*
 * 18fx455
 *
 * EEPROM: 0x00 - 0xff (special, currently unsupported)
 *
 * Program memory: 0x0000 - 0x5fff
 *
 * ID: 0x200000 - 0x200007
 *
 * Config: 0x300000 - 0x30000d (byte-writable)
 *
 * Devid: 0x3ffffe - 0x3fffff (read-only)
 */


/*
 * 18fx550 (default)
 *
 * EEPROM: 0x00 - 0xff (special, currently unsupported)
 *
 * Program memory: 0x0000 - 0x7fff
 *
 * ID: 0x200000 - 0x200007
 *
 * Config: 0x300000 - 0x30000d (byte-writable)
 *
 * Devid: 0x3ffffe - 0x3fffff (read-only)
 */


#ifndef DEV_18FX455
#define DEV_18FX550
#endif /* DEV_18FX455 */

#define MI_EEPROM_BASE      0x00
#define MI_EEPROM_TOP       0xff

#define MI_PROGRAM_BASE   0x0000
#ifdef DEV_18FX455
#define MI_PROGRAM_TOP    0x5fff
#endif /* DEV_18FX455 */
#ifdef DEV_18FX550
#define MI_PROGRAM_TOP    0x7fff
#endif /* DEV_18FX550 */

#define MI_ID_BASE      0x200000
#define MI_ID_TOP       0x200007

#define MI_CONFIG_BASE  0x300000
#define MI_CONFIG_TOP   0x30000d

#define MI_DEVID_BASE   0x3ffffe
#define MI_DEVID_TOP    0x3fffff



typedef unsigned char mi_byte_t;



typedef struct _mi_patch {
  unsigned long base;
  unsigned long top;
  mi_byte_t *contents;
  char *mask;
} mi_patch;

typedef struct _mi_image {
  mi_patch *program;
  mi_patch *id;
  mi_patch *config;
  mi_patch *devid;
  mi_patch *eeprom;
} mi_image;



mi_image *mi_load_hexfile(char *filename);

#endif /* __MEMIMG_H__ */
