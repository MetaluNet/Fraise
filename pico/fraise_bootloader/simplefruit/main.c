/**
 * Blink example copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Modifications copyright (c) 2021 Brian Starkey <stark3y@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define BOARD pico
#include "fraise_device.h"
#include "fraise_eeprom.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

void setup() {
	//fraise_setID(60);
}

int ledPeriod = 250;

void loop(){
	static absolute_time_t nextLed;// = make_timeout_time_ms(100);
	static bool led = false;

	fraise_poll_rx();
	if(absolute_time_min(nextLed, get_absolute_time()) == nextLed) {
		gpio_put(LED_PIN, led = !led);
		nextLed = make_timeout_time_ms(ledPeriod);
	}
}

void fraise_receivebytes(char *data, uint8_t len){
	//printf("receive bytes: "); for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]); putchar('\n');
	if(data[0] == 1) ledPeriod = (int)data[1] * 10;
	else if(data[0] == 2) {
		int rxpin, txpin, drvpin;
		fraise_get_pins(&rxpin, &txpin, &drvpin);
		printf("pins %d %d %d\n", rxpin, txpin, drvpin);
	}
	else if(data[0] == 3) {
		printf("name %s\n", eeprom_get_name());
	}
	else if(data[0] == 4) {
		printf("id %d\n", eeprom_get_id());
	}
}

void fraise_receivechars(char *data, uint8_t len){
	//printf("receive chars: %s\n", data);
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

void fraise_receivebytes_broadcast(char *data, uint8_t len){
	printf("receive broadcast bytes: "); for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]); putchar('\n');
}

void fraise_receivechars_broadcast(char *data, uint8_t len){
	printf("receive broadcast chars: %s\n", data);
}



