/*********************************************************************
 *               KT403 master example for Versa1.0
 *	Connect KT403 through:
 *      AUXSERIAL_TX (K11 for Versa1.0)
 *      AUXSERIAL_RX (K12 for Versa1.0)
 *********************************************************************/
 
#define BOARD Versa1
#include <fruit.h>
#include <kt403.h>

//----------- Setup ----------------
void setup(void) {	
	fruitInit();			
	kt403_Init();        // init DMX master module
}

// ---------- Main loop ------------
byte wasPlaying;

void loop() {
	byte isPlaying;
	fraiseService();// listen to Fraise events
	
	isPlaying = kt403_IsPlaying();
	if(wasPlaying != isPlaying) {
		printf("C playing %d\n", isPlaying);
		wasPlaying = isPlaying;
	}
}

// ---------- Interrupts -----------
void lowInterrupts()
{
	kt403_lowISR();
}

// ---------- Receiving ------------
void fraiseReceive() // receive raw bytes
{
	unsigned char c=fraiseGetChar(); // get first byte
	switch(c) {
		case 40: 
			kt403_SpecifyfolderPlay(fraiseGetChar(), fraiseGetChar()); // (folder, index)
			break;
		case 41: 
			kt403_SetVolume(fraiseGetChar());
			break;
	}
	
}

