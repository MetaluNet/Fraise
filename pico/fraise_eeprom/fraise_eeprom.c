/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "pico/sync.h"
#include "string.h"

#include "fraise_eeprom.h"

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 1024
#endif

#if EEPROM_SIZE > 4095
#error EEPROM_SIZE too big!
#endif

#define EEPROM_NAME_MAX_LENGTH 16
#define EEPROM_ID_LOC 16
#define EEPROM_USER_START 20

static char eeprom_live[EEPROM_SIZE];
extern const char __eeprom_start__;
static const char *eeprom_const = &__eeprom_start__;
static critical_section_t critsec;
static bool initialized = false;

void eeprom_setup() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
	memcpy(eeprom_live, eeprom_const, EEPROM_SIZE);
#pragma GCC diagnostic pop
	critical_section_init(&critsec);
	initialized = true;
}

char eeprom_read(int address) {
	if(address < 0 || address >= EEPROM_SIZE) return 0;
	return eeprom_live[address];
}

void eeprom_write(int address, char data) {
	if(address < 0 || address >= EEPROM_SIZE) return;
	eeprom_live[address] = data;
}

char eeprom_user_read(int address) {
	return eeprom_read(address + EEPROM_USER_START);
}

void eeprom_user_write(int address, char data) {
	if(address < 0) return;
	eeprom_write(address + EEPROM_USER_START, data);
}

void eeprom_write_name(char *name) {
	if(strlen(name) > EEPROM_NAME_MAX_LENGTH) return;
	for(int i = 0; i < EEPROM_NAME_MAX_LENGTH; i++) {
		eeprom_write(i, name[i]);
		if(name[i] == 0) break;
	}
}

const char *eeprom_get_name() {
	return eeprom_const;
}

void eeprom_set_id(uint8_t newid) {
	if(newid <= 0 || newid > 126) return; // 0 and 127 are reserved. More than 127 is an error.
	eeprom_write(EEPROM_ID_LOC, newid);
}

uint8_t eeprom_get_id() {
	return eeprom_read(EEPROM_ID_LOC);
}

void eeprom_commit() {
	if(!initialized) return;
	critical_section_enter_blocking(&critsec);
	flash_range_erase((intptr_t)eeprom_const - (intptr_t)XIP_BASE, 4096);
	flash_range_program((intptr_t)eeprom_const - (intptr_t)XIP_BASE, (const uint8_t *)eeprom_live, EEPROM_SIZE);
	critical_section_exit(&critsec);
}

// --- user utilities ---
static enum {LOAD, SAVE} eeprom_mode;
static int eeprom_count;

void eeprom_declare_data(char *data, uint8_t size) {
	switch(eeprom_mode) {
		case LOAD:
			for(int i = 0; i < size ; i++) data[i] = eeprom_user_read(eeprom_count++);
			break;
		case SAVE:
			for(int i = 0; i < size ; i++) eeprom_user_write(eeprom_count++, data[i]);
			break;
	}
}

void eeprom_load() {
	eeprom_mode = LOAD;
	eeprom_count = 0;
	eeprom_declare_main();
}

void eeprom_save() {
	eeprom_mode = SAVE;
	eeprom_count = 0;
	eeprom_declare_main();
	eeprom_commit();
}




