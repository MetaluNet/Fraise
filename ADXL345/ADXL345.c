/*********************************************************************
 *
 *                ADXL345 for Fraise
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  mar 2017     Original.
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
#include <fruit.h>
#include <i2c_master.h>
#include <ADXL345.h>

static void writeTo(byte address, byte reg, byte value)
{
	i2cm_begin(address,0);
	i2cm_writechar(reg);
	i2cm_writechar(value);
	i2cm_stop();
}

void ADXL345Init(ADXL345 *dev, byte address)
{
	if(address == 0) address = 0x1D;
	else address = 0x53;
	
	dev->address = address;
	
	//ADXL345 TURN ON
	writeTo(address, ADXL345_POWER_CTL, 0);  // Wakeup     
	writeTo(address, ADXL345_POWER_CTL, 16); // Auto_Sleep
	writeTo(address, ADXL345_POWER_CTL, 8);  // Measure
//	writeTo(address, ADXL345_DATA_FORMAT, 0b00001000); // full range
	writeTo(address, ADXL345_DATA_FORMAT, 0b00001011); // full range
}

#define GETACC(which) which##h = i2cm_readchar(); i2cm_ack(); which##l = i2cm_readchar()

void ADXL345Service(ADXL345 *dev)
{
	byte address = dev->address;
	
	i2cm_begin(address, 0);
	i2cm_writechar(ADXL345_DATAX0);
	i2cm_stop();

	i2cm_begin(address, 1);
	GETACC(dev->x); i2cm_ack();
	GETACC(dev->y); i2cm_ack();
	GETACC(dev->z);
	i2cm_stop();
}

void ADXL345Send(ADXL345 *dev, byte prefix)
{
	byte buf[9];

	buf[0] = 'B';
	buf[1] = prefix;
	
	buf[2] = dev->xh;
	buf[3] = dev->xl;
	buf[4] = dev->yh;
	buf[5] = dev->yl;
	buf[6] = dev->zh;
	buf[7] = dev->zl;

	buf[8] = '\n';
	fraiseSend(buf,9);
}

