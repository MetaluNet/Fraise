/*********************************************************************
 *               servo example for Versa1.0
 *	Output RC servo waveforms to K11 (channel 0) and K12 (channel 1).
 *********************************************************************/
#define BOARD Versa1
#include <fruit.h>
#include <servo.h>

//----------- Setup ----------------
void setup(void) {	
	fruitInit();			
	servoInit();        // init servo module
	servoSelect(0,K11); // assign connector K11 to servo channel 0
	servoSelect(1,K12); // assign connector K12 to servo channel 1
}

// ---------- Main loop ------------
void loop() {
	fraiseService();// listen to Fraise events
	servoService();	// servo management routine
}

// ---------- Interrupts ------------
void highInterrupts()
{
	servoHighInterrupt();
}

// ---------- Receiving ------------
void fraiseReceive() // receive raw bytes
{
	unsigned char c=fraiseGetChar();
	if(c==20) servoReceive(); // if first byte is 20, then call servo receive function.
}

