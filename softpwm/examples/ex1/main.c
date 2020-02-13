/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <softpwm.h>

t_delay mainDelay;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- softpwm setup ----------------
	softpwmInit();		// init softpwm module
}

// ---------- Main loop ------------
void loop() {
	fraiseService();	// listen to Fraise events
}

// ---------- Interrupts ------------
void highInterrupts()
{
	softpwmHighInterrupt();
}
void lowInterrupts()
{
	softpwmLowInterrupt();
}

// Receiving

void fraiseReceive() // receive raw bytes
{
	unsigned char c=fraiseGetChar();
	if(c==50) softpwmReceive(); // if first byte is 50, then call softpwm receive function.
}

