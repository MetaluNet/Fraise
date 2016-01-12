/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <analog.h>

unsigned char period = 200; // number of 5 ms time slices between prints of "Hello"

void main(void)
{	
	unsigned char t = 0, t2 = 0;
	t_delay mainDelay;
	
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

// ---------- Main loop ------------
	while(1){
		fraiseService();	// listen to Fraise events
		analogService();	// analog management routine

		if(delayFinished(mainDelay)) // when mainDelay triggers :
		{
			analogSend();		// send analog channels that changed

			delayStart(mainDelay, 5000); 	// re-init mainDelay
			t = t + 1;			// increment period counter	
			if(t >= period){		// if counter overflows :
				t = 0;				 // clear counter
				t2 = t2 + 1;			 // increment aux counter 
				printf("Cs Hello ! t2=%d\n", t2);// print Hello + aux counter value
			}
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
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
}

void fraiseReceive() // receive raw bytes
{
	unsigned char c;
	
	c=fraiseGetChar();	// get the first byte

	switch(c) {
		PARAM_CHAR(1,period); break; 	// if the first byte is 1 then set period 
						//to the value of the next byte
	}
}

void fraiseReceiveCharBroadcast(){} // receive broadcast text
void fraiseReceiveBroadcast(){} // receive broadcast raw bytes


// Interrupt routines
void highInterrupts(){}
void lowInterrupts(){}

// EEPROM parameters management (see eeparams module doc)
void EEdeclareMain(){}

