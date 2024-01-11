/**
 * Blink example copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Modifications copyright (c) 2021 Brian Starkey <stark3y@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define BOARD pico
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"
#include <stdio.h>
#include <string.h>
#include "fraise_device.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

#if 1
void reboot() {
	hw_clear_bits(&watchdog_hw->ctrl, WATCHDOG_CTRL_ENABLE_BITS);
	watchdog_reboot(0, 0, 0);

	while (1) {
		tight_loop_contents();
	}
}


uint8_t lineBuf[256];
uint8_t lineLen;

#define startsWith(str, prefix) (!(strncmp((const char *)(str), (const char *)(prefix), strlen(prefix))))
extern int __fraise_bootloader_start__;

void processLine() {
	if(startsWith(lineBuf, "reboot")) {
		sleep_ms(50); // wait for the host to disconnect the USB device
		reboot();
	}
	else if(startsWith(lineBuf, "whoami")) {
		printf("whoami: simplefruit\n");
	}
	else if(startsWith(lineBuf, "bootload")) {
		sleep_ms(50); // wait for the host to disconnect the USB device
		switch_to_bootloader();
	}
	else if(startsWith(lineBuf, "startbootload")) {
		printf("fraise_bld:%#x \n", (int)&__fraise_bootloader_start__);
	}
}

void stdioTask(void* unused)
{
	int c;
	static bool led;
	//unused; // don't warn
	while((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT){
		gpio_put(LED_PIN, led = !led);
		if(c == '\n') {
			lineBuf[lineLen] = 0;
			processLine();
			lineLen = 0;
		}
		else lineBuf[lineLen++] = c;
	}
}

#endif

void loop();

void loop();
void setup();
int main() {
	stdio_init_all();
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	setup();
	while(true) {
		stdioTask(NULL);
		loop();
	}
	//reboot();
}

void setup() {
	fraise_setup();
	fraise_setID(60);
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
	else if(data[0] == 2) printf("test\n");
}

void fraise_receivechars(char *data, uint8_t len){
	//printf("receive chars: %s\n", data);
	//printf("receive chars: "); for(int i = 0; i < len; i++) { if(data[i] != 0) putchar(data[i]);} putchar('\n');
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

void fraise_receivebytes_broadcast(char *data, uint8_t len){
	//printf("receive broadcast bytes: "); for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]); putchar('\n');
}

void fraise_receivechars_broadcast(char *data, uint8_t len){
	//printf("receive broadcast chars: %s\n", data);
	//printf("receive broadcast chars: "); for(int i = 0; i < len; i++) {if(data[i]) putchar(data[i]);} putchar('\n');
}



