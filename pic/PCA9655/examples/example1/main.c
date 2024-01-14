/*********************************************************************
 *               analog example for Versa2.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <i2c_master.h>
#include <PCA9655.h>

t_delay mainDelay;
PCA9655 pca1;
byte old_buttons;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

	// setup I2C master
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC / 400000 / 4 - 1) ;	//400kHz
	// setup PCA9655
	PCA9655_Config(&pca1, PCA9655_GND_GND_VDD, 255, 0); 	// port1 = all inputs, port2 = all outputs
}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		PCA9655_Read(&pca1, 1);
		if(old_buttons != pca1.in1._byte) {
			old_buttons = pca1.in1._byte;
			printf("CP %d\n", pca1.in1._byte);
		}
		delayStart(mainDelay, 5000); 	// re-init mainDelay
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

void fraiseReceive()
{
	unsigned char c = fraiseGetChar();
	
	if(c == 1) {
		pca1.out2._byte = fraiseGetChar();
		PCA9655_Write(&pca1, 2);
	}
}
