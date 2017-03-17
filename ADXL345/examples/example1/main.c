/*********************************************************************
 *               ADXL345 example for Versa1.0
 *	SCL is K5 and SDA is K7.
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <ADXL345.h>
#include <i2c_master.h>

t_delay mainDelay;
ADXL345 adxl1;
ADXL345 adxl2;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED);		// set the LED pin mode to digital out
	digitalClear(LED);			// clear the LED
	delayStart(mainDelay, 5000);// init the mainDelay to 5 ms

	// setup I2C master:
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
	
	// setup ADXL345:
	ADXL345Init(&adxl1, 0); // 1st ADXL345's SDO pin is high voltage level
	ADXL345Init(&adxl2, 1); // 2nd ADXL345's SDO pin is low voltage level
}


byte axlChan = 0;

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	ADXL345Service(&adxl1);
	fraiseService();	// listen to Fraise events
	ADXL345Service(&adxl2);

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		if(axlChan == 0) {
			ADXL345Send(&adxl1, 1);
			axlChan = 1;
		}
		else {
			ADXL345Send(&adxl2, 2);
			axlChan = 0;
		}
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
}

