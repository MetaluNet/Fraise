/*********************************************************************
 *               dimmer example for Versa1.0
 *	Connect dimmer board to Versa1's VNH2 port.
 *********************************************************************/
#define BOARD Versa1
#include <fruit.h>
#include <dimmer.h>
t_delay mainDelay;

//----------- Setup ----------------
void setup(void) {	
	fruitInit();			
	dimmerInit();        // init dimmer module
	delayStart(mainDelay, 20000); 	// init the mainDelay to 20 ms
}

// ---------- Main loop ------------
void loop() {
	fraiseService();// listen to Fraise events
	dimmerService();	// dimmer management routine
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 20000); 	// re-init mainDelay
		//dimmerPrintDebug();		// 
	}
}

// ---------- Interrupts ------------
void highInterrupts()
{
	dimmerHighInterrupt();
}
void lowInterrupts()
{
	dimmerLowInterrupt();
}

// ---------- Receiving ------------
void fraiseReceive() // receive raw bytes
{
	unsigned char c=fraiseGetChar();
	if(c==40) dimmerReceive(); // if first byte is 40, then call dimmer receive function.
}
