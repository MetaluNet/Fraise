/*********************************************************************
 *               switch example for Versa1.0
 *	Switch capture on connectors K1, K2, K3 and K4. 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <switch.h>

t_delay mainDelay;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- Switch setup ----------------
	switchInit();		// init switch module
	switchSelect(0,K1);	// assign connector K1 to switch channel 0
	switchSelect(1,K2);
	switchSelect(2,K3);
	switchSelect(3,K4);
}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	switchService();	// switch management routine

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// reinit the mainDelay to 5 ms
		switchSend();			// send switch channels that changed
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
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
}

