/**
 * Blink example copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Modifications copyright (c) 2021 Brian Starkey <stark3y@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"
#include <stdio.h>
#include <string.h>
#include "fraise_device.h"
#include "smooth_pwm.h"

#define FRAISE_RX_PIN  0
#define FRAISE_TX_PIN  1
#define FRAISE_DRV_PIN 2

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

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
void processSysLine() {
	switch(lineBuf[1]) {
		case 'R': printf("sID01\n"); break;
		case 'V': printf("sV UsbFraise PicoPied v0.1\n"); break;
		case 'E': puts((const char*)(lineBuf + 2)); break;
	}
}

void processLine() {
	if(lineBuf[0] == '#') processSysLine();
	else if(startsWith(lineBuf, "waitack")) printf("ack\n");
	else if(startsWith(lineBuf, "reboot")) {
		sleep_ms(50); // wait for the host to disconnect the USB device
		reboot();
	}
	else if(startsWith(lineBuf, "readflash")) {
		uint32_t addr;
		sscanf((const char *)lineBuf, "readflash %ld", &addr);
		printf("readflash at %#08lx:", addr);
		for(int i = 0; i < 16; i++) {
			printf("%02X", *(const uint8_t *) (XIP_BASE + addr + i));
		}
		printf("\n");
	}
	else if(startsWith(lineBuf, "getsizeof")) {
		printf("sizeof int=%d long=%d float=%d double=%d\n", sizeof(int), sizeof(long), sizeof(float), sizeof(double));
	}
	else if(startsWith(lineBuf, "testsend")) {
		fraise_puts("hello world!");
	}
	else if(startsWith(lineBuf, "sendbytes")) {
		char buf[] = {0, 1, 2, 3};
		fraise_putbytes(buf, 4);
	}
	else if(startsWith(lineBuf, "printnexttx")) {
		fraise_debug_print_next_txmessage();
	}
	else if(startsWith(lineBuf, "irqcount")) {
		printf("irqcount %ld %ld\n", fraise_debug_get_irq_count(), fraise_debug_get_irq_rx_count());
	}
	else if(startsWith(lineBuf, "getunique")) {
		char buf[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
		pico_get_unique_board_id_string(buf, sizeof(buf));
		printf("uniquestr %s\n", buf);
	}
}

void stdioTask(void* unused)
{
	int c;
	static bool led;
	//unused; // don't warn
	while((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT){
		//gpio_put(LED_PIN, led = !led);
		if(c == '\n') {
			lineBuf[lineLen] = 0;
			processLine();
			lineLen = 0;
		}
		else lineBuf[lineLen++] = c;
	}
}

void loop();
void setup();
int main() {
	stdio_init_all();
	//while(!stdio_usb_connected()); // wait until USB connection

	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	setup();
	while(true) {
		stdioTask(NULL);
		loop();
	}
	reboot();
}

#define NUM_PWM 12
smooth_pwm_t pwms[NUM_PWM];

void setup() {
	fraise_setup(FRAISE_RX_PIN, FRAISE_TX_PIN, FRAISE_DRV_PIN, true);
	fraise_setID(60);
	for(int i = 0; i < NUM_PWM; i++) smooth_pwm_init(&pwms[i], i + 3);
}

void loop(){
	static absolute_time_t next;// = make_timeout_time_ms(100);
	static absolute_time_t nextpwm;
	static bool led = false;

	//fraise_poll_rx(); // call fraise_poll_rx() if fraise_setup() was called with background_rx=false.

	if(absolute_time_min(nextpwm, get_absolute_time()) == nextpwm) {
		for(int i = 0; i < NUM_PWM; i++) smooth_pwm_tick(&pwms[i]);
		nextpwm = make_timeout_time_ms(5);
	}

	if(absolute_time_min(next, get_absolute_time()) == next) {
		gpio_put(LED_PIN, led = !led);
		next = make_timeout_time_ms(100);
	}
}

void fraise_receivebytes(char *data, uint8_t len){
	//printf("receive bytes: "); for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]); putchar('\n');
	if(data[0] == 10 && len > 3) {
		int p = data[1];
		int count = 2;
		while(count + 1 < len) {
			float val = (((int)data[count] << 8) + data[count + 1]) / 65536.0;
			//val = (val * val) * 65535;
			if(p < NUM_PWM) smooth_pwm_goto(&pwms[p], val);
			//printf("pwm[%d]=%f : d0=%d d1=%d\n", p, val, data[count], data[count + 1]);
			p++;
			count += 2;
		}
	}
	else if(data[0] == 11 && len > 3) {
		int p = data[1];
		int count = 2;
		while(count + 1 < len) {
			float smooth = (((int)data[count] << 8) + data[count + 1]) / 65536.0;
			if(p < NUM_PWM) smooth_pwm_set_smooth(&pwms[p], smooth);
			p++;
			count += 2;
		}
	}
	else if(data[0] == 12 && len > 2) {
			float val = (((int)data[1] << 8) + data[2]) / 65536.0;
			//val = (val * val) * 65535;
			for(int i = 0; i < NUM_PWM; i++) smooth_pwm_goto(&pwms[i], val);
	}
	else if(data[0] == 13 && len > 2) {
			float smooth = (((int)data[1] << 8) + data[2]) / 65536.0;
			for(int i = 0; i < NUM_PWM; i++) smooth_pwm_set_smooth(&pwms[i], smooth);
	}

}

void fraise_receivechars(char *data, uint8_t len){
	printf("receive chars: %s\n", data);
}

void fraise_receivebytes_broadcast(char *data, uint8_t len){
	//printf("broadcast bytes: "); for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]); putchar('\n');
}

void fraise_receivechars_broadcast(char *data, uint8_t len){
	//printf("broadcast chars: %s\n", data);
}

