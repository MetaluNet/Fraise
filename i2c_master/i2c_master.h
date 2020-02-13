/*********************************************************************
 *
 *                i2c master library for Fraise pic18f  device
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  october 2013   adapted from Vangelis Rokas's sdcc i2c lib.
 ********************************************************************/

/*
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
*/

#ifndef __I2CM_H__
#define __I2CM_H__

#include <config.h>

#define _I2CPARAM_SPEC	//__data


/* I2C modes of operation */
#define I2C_SLAVE10B_INT	0x0f
#define I2C_SLAVE7B_INT		0x0e
#define I2C_SLAVE_IDLE		0x0b
#define I2C_MASTER		0x08
#define I2C_SLAVE10B		0x07
#define I2C_SLAVE7B		0x06


/* slew rate control */
#define I2C_SLEW_OFF	0x80
#define I2C_SLEW_ON	0x00

/* stop */
void i2cm_stop(void);

/* start */
void i2cm_start(void);

/* restart */
void i2cm_restart(void);

/* not acknowledge */
void i2cm_nack(void);

/* acknowledge */
void i2cm_ack(void);

/* wait until I2C goes idle */
void i2cm_idle(void);

/* is character ready in I2C buffer ?? */
unsigned char i2cm_drdy(void);

/* read a character from I2C module */
unsigned char i2cm_readchar(void);

/* read a string from I2C module */
char i2cm_readstr(_I2CPARAM_SPEC unsigned char *ptr, unsigned char len);

/* write a character to I2C module */
char i2cm_writechar(unsigned char dat);

/* write a string to I2C module */
char i2cm_writestr(unsigned char *ptr);

/* begin communication to I2C module */
char i2cm_begin(unsigned char address, unsigned char doread);

/* configure I2C port for operation */
void i2cm_init(unsigned char mode, unsigned char slew, unsigned char addr_brd);

void i2cm_close(void);

extern char i2cm_errno;
#endif	/* __I2C_H__ */
