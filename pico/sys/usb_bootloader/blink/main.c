/**
 * Simple fruit
 */

#define BOARD pico

#include <stdio.h>
#include <string.h>
#include "fraise.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
int ledPeriod = 250;

/*void setup() {
    gpio_put(LED_PIN, 1);
}*/

void loop(){
	static absolute_time_t nextLed;// = make_timeout_time_ms(100);
	static bool led = false;

	if(time_reached(nextLed)) {
		//if(absolute_time_min(nextLed, get_absolute_time()) == nextLed) {
		gpio_put(LED_PIN, led = !led);
		nextLed = make_timeout_time_ms(ledPeriod);
	}
}

void fraise_receivebytes(const char *data, uint8_t len){
	if(data[0] == 1) ledPeriod = (int)data[1] * 10;	
	else {
		printf("rcvd ");
		for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]);
		putchar('\n');
	}
}

void fraise_receivechars(const char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

