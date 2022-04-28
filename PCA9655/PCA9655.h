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

// Adress for A2_A1_A0 setting:
#define PCA9655_GND_SCL_GND (0x20)>>1
#define PCA9655_GND_SCL_VDD (0x22)>>1
#define PCA9655_GND_SDA_GND (0x24)>>1
#define PCA9655_GND_SDA_VDD (0x26)>>1
#define PCA9655_VDD_SCL_GND (0x28)>>1
#define PCA9655_VDD_SCL_VDD (0x2A)>>1
#define PCA9655_VDD_SDA_GND (0x2C)>>1
#define PCA9655_VDD_SDA_VDD (0x2E)>>1
#define PCA9655_GND_SCL_SCL (0x30)>>1
#define PCA9655_GND_SCL_SDA (0x32)>>1
#define PCA9655_GND_SDA_SCL (0x34)>>1
#define PCA9655_GND_SDA_SDA (0x36)>>1
#define PCA9655_VDD_SCL_SCL (0x38)>>1
#define PCA9655_VDD_SCL_SDA (0x3A)>>1
#define PCA9655_VDD_SDA_SCL (0x3C)>>1
#define PCA9655_VDD_SDA_SDA (0x3E)>>1
#define PCA9655_GND_GND_GND (0x40)>>1
#define PCA9655_GND_GND_VDD (0x42)>>1
#define PCA9655_GND_VDD_GND (0x44)>>1
#define PCA9655_GND_VDD_VDD (0x46)>>1
#define PCA9655_VDD_GND_GND (0x48)>>1
#define PCA9655_VDD_GND_VDD (0x4A)>>1
#define PCA9655_VDD_VDD_GND (0x4C)>>1
#define PCA9655_VDD_VDD_VDD (0x4E)>>1
#define PCA9655_GND_GND_SCL (0x50)>>1
#define PCA9655_GND_GND_SDA (0x52)>>1
#define PCA9655_GND_VDD_SCL (0x54)>>1
#define PCA9655_GND_VDD_SDA (0x56)>>1
#define PCA9655_VDD_GND_SCL (0x58)>>1
#define PCA9655_VDD_GND_SDA (0x5A)>>1
#define PCA9655_VDD_VDD_SCL (0x5C)>>1
#define PCA9655_VDD_VDD_SDA (0x5E)>>1
#define PCA9655_SCL_SCL_GND (0xA0)>>1
#define PCA9655_SCL_SCL_VDD (0xA2)>>1
#define PCA9655_SCL_SDA_GND (0xA4)>>1
#define PCA9655_SCL_SDA_VDD (0xA6)>>1
#define PCA9655_SDA_SCL_GND (0xA8)>>1
#define PCA9655_SDA_SCL_VDD (0xAA)>>1
#define PCA9655_SDA_SDA_GND (0xAC)>>1
#define PCA9655_SDA_SDA_VDD (0xAE)>>1
#define PCA9655_SCL_SCL_SCL (0xB0)>>1
#define PCA9655_SCL_SCL_SDA (0xB2)>>1
#define PCA9655_SCL_SDA_SCL (0xB4)>>1
#define PCA9655_SCL_SDA_SDA (0xB6)>>1
#define PCA9655_SDA_SCL_SCL (0xB8)>>1
#define PCA9655_SDA_SCL_SDA (0xBA)>>1
#define PCA9655_SDA_SDA_SCL (0xBC)>>1
#define PCA9655_SDA_SDA_SDA (0xBE)>>1
#define PCA9655_SCL_GND_GND (0xC0)>>1
#define PCA9655_SCL_GND_VDD (0xC2)>>1
#define PCA9655_SCL_VDD_GND (0xC4)>>1
#define PCA9655_SCL_VDD_VDD (0xC6)>>1
#define PCA9655_SDA_GND_GND (0xC8)>>1
#define PCA9655_SDA_GND_VDD (0xCA)>>1
#define PCA9655_SDA_VDD_GND (0xCC)>>1
#define PCA9655_SDA_VDD_VDD (0xCE)>>1
#define PCA9655_SCL_GND_SCL (0xE0)>>1
#define PCA9655_SCL_GND_SDA (0xE2)>>1
#define PCA9655_SCL_VDD_SCL (0xE4)>>1
#define PCA9655_SCL_VDD_SDA (0xE6)>>1
#define PCA9655_SDA_GND_SCL (0xE8)>>1
#define PCA9655_SDA_GND_SDA (0xEA)>>1
#define PCA9655_SDA_VDD_SCL (0xEC)>>1
#define PCA9655_SDA_VDD_SDA (0xEE)>>1

