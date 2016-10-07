/*********************************************************************
 *               hx711 example for Versa1.0
 *********************************************************************/
#define BOARD Versa1
#include <fruit.h>
#include <hx711.h>

//----------- Setup ----------------
void setup(void) {	
	fruitInit();			
	hx711Init(0);        // init hx711 module, gainA=128
}

// ---------- Main loop ------------
void loop() {
	fraiseService();// listen to Fraise events
	hx711Service(); // hx711 service routine
}

// ---------- Receiving ------------
void fraiseReceive() // receive raw bytes
{
	int i;
	unsigned char c=fraiseGetChar();    // get first byte
	if(c == 50) {                       // if first byte is 50 
	    c=fraiseGetChar();              // read channel byte
	    printf("C 50 %d %ld\n", c, hx711Read(c)); // return selected channel current value
	}
}

