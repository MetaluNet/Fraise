/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include <stdio.h>
#include <string.h>
#include "fraise_bootdevice.h"
#include "bootloader.h"
#include "fraise_eeprom.h"

#include "boardconfig.h"

#ifdef FRAISE_BLD_DEBUG
#define DEBUG printf
#else
#define DEBUG
#endif

static const uint LED_PIN = PICO_DEFAULT_LED_PIN;

uint8_t lineBuf[256];
uint8_t lineLen;

bool isVerified = false;

static inline bool timed_out(absolute_time_t t) { return (absolute_time_min(t, get_absolute_time()) == t); }
absolute_time_t fraiseTimeout;
static inline void fraiseResetTimeout() { fraiseTimeout = make_timeout_time_ms(1000); }

#define startsWith(str, prefix) (!(strncmp((const char *)(str), (const char *)(prefix), strlen(prefix))))

#ifdef FRAISE_BLD_DEBUG
void processLine() {
	if(startsWith(lineBuf, "#R")) printf("sID02\n");
	else if(startsWith(lineBuf, "#E")) puts((const char*)(lineBuf + 2));
	else if(startsWith(lineBuf, "#V")) printf("sV UsbFraise PicoPied v0.1\n");
	else if(startsWith(lineBuf, "runapp")) run_app();
	else if(startsWith(lineBuf, "waitack")) printf("ack\n");
	else if(startsWith(lineBuf, "reboot")) {
		sleep_ms(50); // wait for the host to disconnect the USB device
		reboot();
	}
	else if(startsWith(lineBuf, "whoami")) {
		printf("whoami: fraise_bootloader\n");
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
}
#endif

void getName() {
	char buf[32];
	int i = 0;
	buf[i++] = 'G';
	const char *c = eeprom_get_name();
	while(*c != 0) buf[i++] = *c++;
	buf[i++] = '\n';
	fraise_puts(buf);
}

void verifyName(char *data, uint8_t len) {
	if(data[1] == '?' && len == 2) {
		getName();
		return;
	}
	isVerified = (strncmp(data + 1, eeprom_get_name(), len - 1) == 0);
	DEBUG("isVerified: %s\n", isVerified?"true":"false");
	if(isVerified) fraise_puts(" V\n");
}

void setName(char *data, uint8_t len) {
	if(strcmp(data, "RENAME:") <= 0) return;
	eeprom_write_name(data + strlen("RENAME:"));
	eeprom_commit();
	fraise_puts(" R\n");
	isVerified = true;
}

void fraiseLineReceived(char *data, uint8_t len) {
	char c = data[0];
	//printf("line: %s\n", data);
	if(c == 'V') verifyName(data, len);
	//else if(c == 'G') getName();
	else if(c == 'A') run_app();
	else if(c == 'R') setName(data, len);
	if(isVerified) {
		if(c == ':') {
			int ret = processHexLine(data, len);
			switch(ret) {
				case 0: fraise_puts(" X\n"); break;
				case 1: fraise_puts(" Y\n"); break;
				case -1: fraise_puts(" z\n"); break;
				case -2: fraise_puts(" u\n"); break;
				case -3: fraise_puts(" l\n"); break;
			}
		}
		//else
	}
}

void fraiseTask() {
	static char line[64];
	static uint8_t lineLen;
	static uint8_t wcount;
	static uint8_t checksum;
	static absolute_time_t lineTimeout;
	uint16_t w;
	#define lineTimedOut() timed_out(lineTimeout)
	#define lineResetTimeout() lineTimeout = make_timeout_time_ms(100)

	while(fraise_getword(&w)) {
		//printf("word: %d \t %c\n", w, w & 127);
		if(w > 255) {
			if( (w & 255) != 0) { run_app();}
			fraiseResetTimeout();
			lineResetTimeout();
			lineLen = wcount = 0;
		} else {
			if(lineTimedOut()) lineLen = wcount = 0;
			lineResetTimeout();
			if(lineLen == 0) {
				checksum = lineLen = w;
				wcount = 0;
			}
			else {
				line[wcount++] = w;
				checksum += w;
				if(wcount == lineLen) {
					if(checksum == 0) {
						line[wcount - 1] = 0;
						fraiseLineReceived(line, lineLen - 1);
					}
					else { DEBUG("checksum error! %d\n", checksum);/* signal checksum error)*/}
					lineLen = wcount = 0;
				}
			}
		}
	}
}

#ifdef FRAISE_BLD_DEBUG
void stdioTask()
{
	int c;
	while((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT){
		if(c == '\n') {
			lineBuf[lineLen] = 0;
			processLine();
			lineLen = 0;
		}
		else lineBuf[lineLen++] = c;
	}
}
#endif

absolute_time_t nextLed;
bool led = false;

int main() {
#ifdef FRAISE_BLD_DEBUG
	stdio_init_all();
	setVerbose(true); // bootloader verbose
#endif
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);
	eeprom_setup();
	fraise_setup(FRAISE_RX_PIN, FRAISE_TX_PIN, FRAISE_DRV_PIN);
	nextLed = make_timeout_time_ms(100);

	fraiseResetTimeout();
	while(true) {
	#ifdef FRAISE_BLD_DEBUG
		stdioTask(NULL);
	#endif
		fraiseTask();
		if(timed_out(nextLed)) {
			gpio_put(LED_PIN, led = !led);
			nextLed = make_timeout_time_ms(100);
		}
	#ifndef FRAISE_BLD_DEBUG
		if(timed_out(fraiseTimeout) && !isVerified) run_app();
	#endif
	}
}

