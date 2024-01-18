/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*#include "pico/stdlib.h"
#include "pico/async_context_threadsafe_background.h"
#include "pico/async_context_poll.h"
#include "hardware/pio.h"
#include "pico/stdio/driver.h"
#include "RP2040.h"
#include "hardware/resets.h"

#include "fraise.pio.h"
#include "fraise_master.h"
#include "fraise_buffers.h"*/
#define _FRAISE_INTERNAL_
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"
#include "fraise_eeprom.h"
#include "fraise_master.h"

uint8_t lineBuf[256];
uint8_t lineLen;
uint8_t piedID = 1;

void reboot() {
	hw_clear_bits(&watchdog_hw->ctrl, WATCHDOG_CTRL_ENABLE_BITS);
	watchdog_reboot(0, 0, 0);

	while (1) {
		tight_loop_contents();
	}
}

uint8_t gethexbyte(const uint8_t *buf)
{
	uint8_t cl, ch;
	ch = buf[0] -'0';
	if(ch > 9) ch += '9' - 'A' + 1;
	cl = buf[1] -'0';
	if(cl > 9) cl += '9' - 'A' + 1;
	return (ch << 4) + cl;
}

void processSysLine() {
	static bool unlocked = false;
	switch(lineBuf[1]) {
		case 'R': printf("sID%02X\n", piedID); break;
		case 'V': printf("sV UsbFraise PicoPied v0.1\n"); break;
		case 'E': puts((const char*)(lineBuf + 2)); break;
		case 'U': if(!strncmp(lineBuf, "#UNLOCK", 7)) unlocked = true; return;
		case 'W': if(unlocked) {
				piedID = gethexbyte(lineBuf + 2);
				eeprom_set_id(piedID);
				eeprom_commit();
			}
			break;
		case 'S': fraise_master_set_poll(gethexbyte(lineBuf + 2), true); break;
		case 'C': fraise_master_set_poll(gethexbyte(lineBuf + 2), false); break;
		case 'i': fraise_master_reset_polls(); break;
		case 'F': fraise_master_stop_bootload(); break;
	}
	unlocked = false;
}

void processBroadcast() {
	switch(lineBuf[1]) {
		case 'N': fraise_master_assign(lineBuf + 4, gethexbyte(lineBuf + 2)); break;
		case 'b': {
				char buf[64];
				uint8_t count = 2;
				uint8_t bufcount = 0;
				while(count < lineLen - 1) {
					buf[bufcount++] = gethexbyte(lineBuf + count);
					count += 2;
				}
				fraise_master_sendbytes_broadcast(buf, bufcount);
			}
			break;
		case 'B': fraise_master_sendchars_broadcast(lineBuf + 2);
		case 'F': fraise_master_start_bootload(lineBuf + 2); break;
	}
}

#define startsWith(str, prefix) (!(strncmp((const char *)(str), (const char *)(prefix), strlen(prefix))))
#define ishex(x) ((x >= '0'&& x <='9') || (x >= 'A' && x <= 'F'))
void processLine() {
	if(lineBuf[0] == '#') processSysLine();
	else if(fraise_master_is_bootloading()) {
		fraise_master_send_bootload(lineBuf);
	} else {
		if(ishex(lineBuf[0]) && ishex(lineBuf[1])) {
			uint8_t id = gethexbyte(lineBuf);
			if(id < 128) {
				char buf[64];
				uint8_t count = 2;
				uint8_t bufcount = 0;
				while(count < lineLen - 1) {
					buf[bufcount++] = gethexbyte(lineBuf + count);
					count += 2;
				}
				fraise_master_sendbytes(id, buf, bufcount);
			}
			else fraise_master_sendchars(id & 127, lineBuf + 2);
		}
		else if(lineBuf[0] == '!') processBroadcast();
		else if(startsWith(lineBuf, "waitack")) printf("ack\n");
		else if(startsWith(lineBuf, "reboot")) {
			sleep_ms(50); // wait for the host to disconnect the USB device
			reboot();
		}
	}
}

void stdioTask()
{
	int c;
	static bool led;
	while((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT){
		if(c == '\n') {
			lineBuf[lineLen] = 0;
			processLine();
			lineLen = 0;
		}
		else lineBuf[lineLen++] = c;
	}
}

int main() {
	stdio_init_all();
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	fraise_setup();
	eeprom_setup();
	setup();
	while(true) {
		stdioTask();
		loop();
	}
}

__attribute__((weak)) void setup(){}
__attribute__((weak)) void loop(){}
#define STRINGIFY(x) #x
#ifdef FRAISE_MASTER_DEBUG
#define dummy_callback(f) __attribute__((weak)) void f(const char *data, uint8_t len){ printf("dummy " STRINGIFY(f) "()\n");}
#else
#define dummy_callback(f) __attribute__((weak)) void f(const char *data, uint8_t len){}
#endif

dummy_callback(fraise_receivebytes);
dummy_callback(fraise_receivechars);
dummy_callback(fraise_receivebytes_broadcast);
dummy_callback(fraise_receivechars_broadcast);

