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

typedef struct {
	byte address;
	BYTE in1, in2, out1, out2;
} PCA9655;

void PCA9655_Config(PCA9655 *dev, byte address, byte conf1, byte conf2);

void PCA9655_Read(PCA9655 *dev, byte which); //which=1:read1 which=2:read2 which=3:read1&2

void PCA9655_Write(PCA9655 *dev, byte which); //which=1:read1 which=2:read2 which=3:read1&2

