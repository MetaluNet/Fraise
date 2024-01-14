/*********************************************************************
 *               i2c slave example for Versa2.0
 *               emulates a 255 bytes i2c ram
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <i2c_slave.h>

//----------- Setup ----------------
void setup(void) {
	fruitInit();
	// setup I2C slave
	i2c_slave_setup(0x10); // set i2c address to 0x10
	i2c_regs[0] = 123;
}

// ---------- Main loop ------------
void loop() {
	fraiseService();	// listen to Fraise events
}

// Interrupts
void lowInterrupts()
{
	i2c_slave_ISR();
}

