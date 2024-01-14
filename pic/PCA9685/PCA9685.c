/*********************************************************************
 *
 *                PCA9685 for Fraise
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  mar 2018     Original.
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
#include <PCA9685.h>

// adapted from:
/*************************************************** 
  This is a library for our Adafruit 16-channel PWM & Servo driver

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815

  These displays use I2C to communicate, 2 pins are required to  
  interface.

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Originaly written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD

static void delay(word millisecs)
{
	t_delay del;
	delayStart(del, millisecs * 1000);
	while(!delayFinished(del)){}
}

static byte read8(byte i2caddr, byte addr)
{
	byte b;
	
	i2cm_begin(i2caddr, 0);
	i2cm_writechar(addr);
	i2cm_stop();

	i2cm_begin(i2caddr, 1);
	b = i2cm_readchar();
	i2cm_stop();
	
	return b;
}

static void write8(byte i2caddr, byte addr, byte value)
{
	i2cm_begin(i2caddr, 0);
	i2cm_writechar(addr);
	i2cm_writechar(value);	
	i2cm_stop();
}

void PCA9685_Init(byte i2caddr) {
  write8(i2caddr, PCA9685_MODE1, 0x80);
  delay(10);
  PCA9685_SetPWMPrescaler(i2caddr, 5); // 1kHz
}


/**************************************************************************/
/*! 
    @brief  Sets the PWM prescaler
    @param  i2caddr  The 7-bit I2C address to locate the PCA9685, default is 0x40
    @param  prescale PWM prescaler = (6103 / f(Hz)) - 1
*/
/**************************************************************************/
void PCA9685_SetPWMPrescaler(byte i2caddr, byte prescale) {
  
  byte oldmode = read8(i2caddr, PCA9685_MODE1);
  byte newmode = (oldmode&0x7F) | 0x10; // sleep
  write8(i2caddr, PCA9685_MODE1, newmode); // go to sleep
  write8(i2caddr, PCA9685_PRESCALE, prescale); // set the prescaler
  write8(i2caddr, PCA9685_MODE1, oldmode);
  delay(5);
  write8(i2caddr, PCA9685_MODE1, oldmode | 0xa0);  //  This sets the MODE1 register to turn on auto increment.
}

/**************************************************************************/
/*! 
    @brief  Sets the PWM output of one of the PCA9685 pins
    @param  i2caddr  The 7-bit I2C address to locate the PCA9685, default is 0x40
    @param  num One of the PWM output pins, from 0 to 15
    @param  value The width of the PWM, from 0 to 4095
*/
/**************************************************************************/
void PCA9685_SetPWM(byte i2caddr, byte num, word value) {
	word on = 0;
	word off = value;
	if (value == 4095) {
		// Special value for signal fully on.
		on = 4096;
		off = 0;
	}
	else if (value == 0) {
		// Special value for signal fully off.
		on = 0;
		off = 4096;
	}

	i2cm_begin(i2caddr, 0);
	i2cm_writechar(LED0_ON_L + 4*num);
	i2cm_writechar(on);	
	i2cm_writechar(on>>8);	
	i2cm_writechar(off);	
	i2cm_writechar(off>>8);	
	i2cm_stop();
}


