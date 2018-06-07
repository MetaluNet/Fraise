/*********************************************************************
 *               Test Fraise on Versa1.0 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>

unsigned char period = 200; // number of 5 ms time slices between prints of "Hello"
unsigned char t = 0, t2 = 0;
t_delay mainDelay;
unsigned toggleLed = 0;
unsigned char currentLed = 0;
unsigned char toclientBuf[4];

void setup(void) {		
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms
	
	pinModeDigitalIn(K1);
}


void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		t = t + 1;			// increment period counter	
		if(t >= period){		// if counter overflows :
			t = 0;				 // clear counter
			t2 = t2 + 1;			 // increment aux counter
			if(toggleLed) {
				currentLed = !currentLed;
				toclientBuf[0] = 'C';
				toclientBuf[1] = 'L';
				/*if(currentLed) toclientBuf[2] = '1';
				else toclientBuf[2] = '0';*/
				toclientBuf[2] = '0' + (currentLed != 0);
				fraiseSendBroadcast(toclientBuf, 3);
			}
		}
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	unsigned char buf[11];
	unsigned char i = 0;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen() - 1; 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
	else if(c=='S') { 	// broadcast text (send it back to broadcast)
		buf[i++] = 'C';
		c = fraiseGetLen() - 1;			// get length of current packet
		if(c > 10) c = 10;
		while(c--) buf[i++] = fraiseGetChar();// send each received byte
		fraiseSendBroadcast(buf, i);
	}	
}

void fraiseReceiveCharBroadcast() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
}

void fraiseReceive() // receive raw bytes
{
	unsigned char c;
	
	c=fraiseGetChar();	// get the first byte

	switch(c) {
		PARAM_CHAR(1,period); break; 	// if the first byte is 1 then set period 
										//to the value of the next byte
		PARAM_CHAR(2,toggleLed); break; 	// if the first byte is 2 then set/clear toggleLed 
	}
}

