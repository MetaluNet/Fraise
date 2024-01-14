/*********************************************************************
 *
 *                I2C slave library for Fraise pic18f device
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  april 2022     Original.
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

#ifndef _I2C_SLAVE_H_
#define _I2C_SLAVE_H_

#include <fruit.h>

void i2c_slave_setup(unsigned char addr);
void i2c_slave_ISR();

#ifndef I2C_REG_SIZE
#define I2C_REG_SIZE 32
#endif
extern unsigned char i2c_regs[I2C_REG_SIZE]; //actual i2c_slave memory storage

#endif // _I2C_SLAVE_H_

