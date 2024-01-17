/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EEPROM_H
#define _EEPROM_H

#ifndef EEPROM_SIZE
#define EEPROM_SIZE 1024
#endif

#if EEPROM_SIZE > 4095
#error EEPROM_SIZE too big!
#endif

#define EEPROM_NAME_MAX_LENGTH 16
#define EEPROM_ID_LOC 16
#define EEPROM_USER_START 20

void eeprom_setup();

char eeprom_read(int address);
void eeprom_write(int address, char data);

void eeprom_write_name(char *name);
const char *eeprom_get_name();

void eeprom_set_id(uint8_t newid);
uint8_t eeprom_get_id();

void eeprom_commit();

#endif // _EEPROM_H
