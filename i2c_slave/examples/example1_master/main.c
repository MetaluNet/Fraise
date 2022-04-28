/*********************************************************************
 *			Versa2 i2c master test for i2c slave example
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <i2c_master.h>

byte slaveAddress = 0x10;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();

	// setup I2C master
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC / 100000 / 4 - 1) ;	//100kHz
}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
}

byte slaveRead(byte reg)
{
	byte val;
	i2cm_begin(slaveAddress, 0);
	i2cm_writechar(reg);
	i2cm_stop();

	i2cm_begin(slaveAddress, 1);
	val = i2cm_readchar();
	i2cm_stop();
	
	return val;
}

void slaveWrite(byte reg, byte val)
{
	i2cm_begin(slaveAddress, 0);
	i2cm_writechar(reg);
	i2cm_writechar(val);
	i2cm_stop();
}

// Receiving

void fraiseReceive()
{
	unsigned char reg, val;
	unsigned char c = fraiseGetChar();
	
	if(c == 1) { // read
		reg = fraiseGetChar();
		val = slaveRead(reg);
		printf("CR %d %d\n", reg, val);
	} else if(c == 2) { // write
		reg = fraiseGetChar();
		val = fraiseGetChar();
		slaveWrite(reg, val);
	}
}


