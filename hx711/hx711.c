/*********************************************************************
 *
 *     Load Cell Amplifier HX711 library for Fraise pic18f device
 *
 *********************************************************************
 * Author               Date        Comment
 *********************************************************************
 * Antoine Rousseau  sept 2016     Original.
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

#include <hx711.h>

static long valA = 0 , valB = 0;
static unsigned char gainA = 0;
static unsigned char currentFront = 0; // chanA=24+2 bits,  chanB=25 or 27 bits  = 51 or 53 bits total = 102 or 106 fronts.

void hx711Init(unsigned char _gainA)
{
	gainA = (_gainA != 0);
	digitalClear(HX711_SCK_PIN);
	pinModeDigitalOut(HX711_SCK_PIN);
	pinModeDigitalIn(HX711_DATA_PIN);
}

static unsigned char shiftIn() // shift HX711_DATA_PIN with HX711_SCK_PIN, MSB first.
{
    unsigned char v = 0;
    unsigned char i = 8;

    while(i--) {
        v <<= 1;
        digitalSet(HX711_SCK_PIN);
        /*nop();
        nop();*/
        digitalClear(HX711_SCK_PIN);
        if(digitalRead(HX711_DATA_PIN)) v += 1;
    }
    return v;
}

static long format(unsigned long in)
{
#if 0
    in = ~in;
    if( ((in & 0x00FF0000) != 0x00800000) && (in != 0xFF7FFFFF))
        in &= 0x00FFFFFF;
    in += 1;
    return (long)in;
#endif
    return in^0x800000;
}

void hx711Service()
{
    static unsigned long tmpVal;
    static unsigned char curBit = 0;
    
    if(((curBit == 0) || (curBit == 25)) && digitalRead(HX711_DATA_PIN)) return;
    if((curBit == 0) || (curBit == 26)) tmpVal = 0;
    
    curBit += 1;
    digitalSet(HX711_SCK_PIN);
    Nop();
    Nop();
    Nop();
    Nop();
    digitalClear(HX711_SCK_PIN);
    
    tmpVal <<= 1;
    if(digitalRead(HX711_DATA_PIN)) tmpVal += 1;
    
    if(curBit == 24) valA = format(tmpVal);
    if(curBit == 50) valB = format(tmpVal);
    
    if((gainA==0) && (curBit == 51)) curBit = 0;
    if(curBit >= 53) curBit = 0;
}

long hx711Read(unsigned char channel)
{
    if(channel == 0) return valA;
    else return valB;


#if 0
	unsigned char i;
    unsigned long value = 0;
    unsigned char data[3] = { 0 };
    unsigned char filler = 0x00;
	
	if(mode>2) mode=2;
	mode += 1;


	// pulse the clock pin 24 times to read the data
    data[2] = shiftIn();
    data[1] = shiftIn();
    data[0] = shiftIn();

	// set the channel and the gain factor for the next reading using the clock pin
	for (i = 0; i < mode; i++) {
		digitalSet(HX711_SCK_PIN);
		digitalClear(HX711_SCK_PIN);
	}

    // Datasheet indicates the value is returned as a two's complement value
    // Flip all the bits
    data[2] = ~data[2];
    data[1] = ~data[1];
    data[0] = ~data[0];

    // Replicate the most significant bit to pad out a 32-bit signed integer
    if ( data[2] & 0x80 ) {
        filler = 0xFF;
    } else if ((0x7F == data[2]) && (0xFF == data[1]) && (0xFF == data[0])) {
        filler = 0xFF;
    } else {
        filler = 0x00;
    }

    /*// Construct a 32-bit signed integer
    value = ( static_cast<unsigned long>(filler) << 24
            | static_cast<unsigned long>(data[2]) << 16
            | static_cast<unsigned long>(data[1]) << 8
            | static_cast<unsigned long>(data[0]) );

    // ... and add 1
    return static_cast<long>(++value);*/
    value = (unsigned long)(filler) << 24
            | ((unsigned long)(data[2]) << 16)
            | ((unsigned long)(data[1]) << 8)
            | ((unsigned long)(data[0]) );
    value += 1;
    
    return value;
#endif

}

/*void hx711Service_()
{
    static unsigned long tmpVal;
    //unsigned char curBit;
    
    if((currentFront == 0) || (currentFront == 54)) tmpVal = 0;
    
    currentFront += 1;
    if(currentFront & 1) {
        digitalSet(HX711_SCK_PIN);
        return;
    }

    digitalClear(HX711_SCK_PIN);
    
    tmpVal <<= 1;
    if(digitalRead(HX711_DATA_PIN)) tmpVal += 1;
    
    if(currentFront == 48) valA = format(tmpVal);
    if(currentFront == 100) valB = format(tmpVal);
    
    if((gainA==0) && (currentFront == 102)) currentFront = 0;
    if(currentFront >= 106) currentFront = 0;
}   */



