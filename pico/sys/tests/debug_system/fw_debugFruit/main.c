/**
 * 12-channel PWM driver
 */

#define BOARD pico

#include <stdio.h>
#include <string.h>
#include "fraise.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
int ledPeriod = 250;

extern void stdioTask();

void setup() {
	gpio_init(0);
	gpio_set_dir(0, GPIO_OUT);
	gpio_put(0, 1);

	gpio_init(12);
	gpio_set_dir(12, GPIO_OUT);
	gpio_put(12, 0);
}

int testval = 10;
void loop(){
	static absolute_time_t nextLed;
	static bool led = false;

	if(time_reached(nextLed)) {
		gpio_put(LED_PIN, led = !led);
		nextLed = make_timeout_time_ms(ledPeriod);
	}
	stdioTask();
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

