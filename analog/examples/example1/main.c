/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <analog.h>

t_delay mainDelay;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- Analog setup ----------------
	analogInit();		// init analog module
	analogSelect(0,K1);	// assign connector K1 to analog channel 0
	analogSelect(1,K2);
	analogSelect(2,K3);
	analogSelect(3,K5);
}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		analogSend();		// send analog channels that changed
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

