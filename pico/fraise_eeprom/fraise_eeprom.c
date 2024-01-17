/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "string.h"

#include "fraise_eeprom.h"

char eeprom_live[EEPROM_SIZE];
extern int __fraise_app_start__, __fraise_app_length__;
extern int __eeprom_start__;
const char *eeprom_const = (char*)&__eeprom_start__;

void eeprom_setup() {
	memcpy(eeprom_live, eeprom_const, EEPROM_SIZE);
}

char eeprom_read(int address) {
	if(address < 0 || address >= EEPROM_SIZE) return 0;
	return eeprom_live[address];
}

void eeprom_write(int address, char data) {
	if(address < 0 || address >= EEPROM_SIZE) return;
	eeprom_live[address] = data;
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
	eeprom_write(EEPROM_ID_LOC, newid);
}

uint8_t eeprom_get_id() {
	return eeprom_read(EEPROM_ID_LOC);
}

void eeprom_commit() {
	flash_range_erase((intptr_t)eeprom_const - (intptr_t)XIP_BASE, 4096);
	flash_range_program((intptr_t)eeprom_const - (intptr_t)XIP_BASE, eeprom_live, EEPROM_SIZE);
}

