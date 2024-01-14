/*********************************************************************
 *                Fraise fruit example
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>

t_delay printDelay;
t_time printPeriod = 500000; 				// default print period to 0.5 second (500000 microsecond)
unsigned char count;

//-------- the setup() function is called once at startup
void setup() {
	fruitInit();							// initialize the board and the Fraise library
	pinModeDigitalOut(LED); 				// set the LED pin to digital out mode
	digitalClear(LED);						// clear the LED
	pinModeDigitalIn(K1);					// set pin K1 to digital input mode
}

// ------- the loop() function is called indefinitely
void loop() {
	fraiseService();						// listen to Fraise events

	if(delayFinished(printDelay))			// when printDelay triggers:
	{
		delayStart(printDelay, printPeriod);// re-start printDelay
		count = count + 1;					// increment count
		printf("Ccount %d\n", count);		// print count value
		printf("CK1 %d\n", digitalRead(K1));// print the state of input pin K1
	}
}

// ------- the fraiseReceiveChar() function is called when a "text" message arrived
void fraiseReceiveChar() 					// receive text message.
{
	unsigned char c = fraiseGetChar();		// get the first character of the message.
	if(c == 'L') {							// if the first char is 'L',
		c = fraiseGetChar();				// get the next one;
		digitalWrite(LED, c != '0');		// if it's the char '0', switch off the LED (otherwide switch it on)
	}
	else if(c == 'E') { 					// if the first char is 'E', echo the message (send it back to host):
		printf("C");						// start a text message
		c = fraiseGetLen(); 				// get the length of the incoming packet
		while(c--) printf("%c", fraiseGetChar());// print each byte of the incoming packet
		putchar('\n');						// append end of line to send the message
	}
}

// ------- the fraiseReceive() function is called when a "raw bytes" message arrived
void fraiseReceive() 						// receive raw bytes
{
	unsigned char c = fraiseGetChar();		// get the first byte of the message
	if(c == 1) {							// if it's equal to 1,
		printPeriod = fraiseGetLong();		// set printPeriod to the next long int in the incoming message.
	}
}

