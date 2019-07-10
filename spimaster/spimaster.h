/*********************************************************************
 *
 *                SPI master library for Fraise pic18f  device
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  nov 2018   (nearly) initial
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
#ifndef SPIMASTER_H
#define SPIMASTER_H
#include <core.h>

#define SPI_BR_FOSC_4 0b0000
#define SPI_BR_FOSC_16 0b0001
#define SPI_BR_FOSC_64 0b0010

#ifndef SPI_BITRATE
#define SPI_BITRATE SPI_BR_FOSC_16
#endif

void SPImaster1Init();
void SPImaster2Init();

byte SPImaster1Transfer(byte b);
byte SPImaster2Transfer(byte b);

#endif

