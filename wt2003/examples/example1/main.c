/*********************************************************************
 *               WT2003 master example for Versa1.0
 *	Connect WT2003 through:
 *      AUXSERIAL_TX (K11 for Versa)
 *      AUXSERIAL_RX (K12 for Versa)
 *********************************************************************/
 
#define BOARD Versa2
#include <fruit.h>
#include <wt2003.h>

//----------- Setup ----------------
void setup(void) {	
	fruitInit();			
	wt2003_Init();        // init wt2003 module
}

// ---------- Main loop ------------
byte wasPlaying;

void loop() {
	byte isPlaying;
	fraiseService();// listen to Fraise events
	
	isPlaying = wt2003_IsPlaying();
	if(wasPlaying != isPlaying) {
		printf("C playing %d\n", isPlaying);
		wasPlaying = isPlaying;
	}
}

// ---------- Interrupts -----------
/*void lowInterrupts()
{
	kt403_lowISR();
}*/

// ---------- Receiving ------------
void fraiseReceive() // receive raw bytes
{
	unsigned char c=fraiseGetChar(); // get first byte
	switch(c) {
		case 40: 
			wt2003_SpecifyMusicPlay(fraiseGetInt());
			break;
		case 41: 
			wt2003_SetVolume(fraiseGetChar());
			break;
		case 42: 
			wt2003_PlayLoop(fraiseGetChar());
			break;
		case 43: 
			wt2003_PlayPause();
			break;
		case 44: 
			wt2003_Stop();
			break;
	}
	
}

