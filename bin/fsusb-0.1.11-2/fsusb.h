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


#ifndef __FSUSB_H__
#define __FSUSB_H__

#include "bootload.h"

typedef struct usb_dev_handle picdem_handle;

void rjl_request_flash(picdem_handle *d, int offset, int len, bl_packet *pack);
void rjl_request_version(picdem_handle *d, unsigned char *r);
//void rjl_write_flash(picdem_handle *d, int offset, int len, byte *data, bl_packet *pack);
//void rjl_wipe_flash(picdem_handle *d);
picdem_handle *rjl_fsusb_open(void);
void rjl_write_block(picdem_handle *d, int offset, byte *data);
//void rjl_erase_block(picdem_handle *d, int offset);
void rjl_write_config_block(picdem_handle *d, int offset, int len, byte *data);

#endif /* __FSUSB_H__ */
