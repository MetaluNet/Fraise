/*********************************************************************
 *
 *                PCA9655 for Fraise
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  oct 2013     Original.
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
#include <core.h>
#include <i2c_master.h>
#include <PCA9655.h>

void PCA9655_Config(PCA9655 *dev, byte address, byte conf1, byte conf2)
{
	if(address) dev->address = address;
	i2cm_begin(address,0);
	i2cm_writechar(6);
	i2cm_writechar(conf1); 
	i2cm_writechar(conf2);
	i2cm_stop();
	dev->out1._byte = dev->out2._byte = 255;	
}

void PCA9655_Read(PCA9655 *dev, byte which) //which=1:read1 which=2:read2 which=3:read1&2
{
	byte address = dev->address;
	
	i2cm_begin(address, 0);
	i2cm_writechar((which & 1) == 0);
	i2cm_stop();

	i2cm_begin(address, 1);
	if(which & 1) {
		dev->in1._byte = i2cm_readchar();
		if(which & 2) i2cm_ack();
	}
	if(which & 2) dev->in2._byte = i2cm_readchar();

	i2cm_stop();
}

void PCA9655_Write(PCA9655 *dev, byte which) //which=1:read1 which=2:read2 which=3:read1&2
{
	byte address = dev->address;
	
	i2cm_begin(address, 0);
	i2cm_writechar(((which & 1) == 0) + 2);

	if(which & 1) i2cm_writechar(dev->out1._byte);	
	if(which & 2) i2cm_writechar(dev->out2._byte);

	i2cm_stop();
}


