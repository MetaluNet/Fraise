/*********************************************************************
 *               dimmer example for Versa1.0
 *	Send triac triggers to K11 (channel 0) and K12 (channel 1).
 *********************************************************************/
#define BOARD Versa1
#include <fruit.h>
#include <dimmer.h>

//----------- Setup ----------------
void setup(void) {	
	fruitInit();			
	dimmerInit();        // init servo module
//	dimmerSelect(0,K11); // assign connector K11 to dimmer channel 0
//	dimmerSelect(1,K12); // assign connector K12 to dimmer channel 1
}

// ---------- Main loop ------------
void loop() {
	fraiseService();// listen to Fraise events
	dimmerService();	// dimmer management routine
}

// ---------- Interrupts ------------
void highInterrupts()
{
	dimmerHighInterrupt();
}

// ---------- Receiving ------------
void fraiseReceive() // receive raw bytes
{
	unsigned char c=fraiseGetChar();
	if(c==40) dimmerReceive(); // if first byte is 40, then call dimmer receive function.
}
