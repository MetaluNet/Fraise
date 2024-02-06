/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_EEPROM_H
#define _FRAISE_EEPROM_H

/** \file fraise_eeprom.h
 *  \defgroup fraise_eeprom fraise_eeprom
 *
 * EEPROM emulation in flash memory
 *
 * Eeprom must first be initialized with eeprom_setup(), than can be read or modified,
 * then (if modified) must be committed with eeprom_commit(). This function is auto-protected
 * with a critical section, which ensures that interrupt are fully disabled during actual flash writing.
 *
 * The fraise_eeprom API reserves some bytes at the beginning of the allocated space, to store fraise system data,
 * such as the "name" and the "ID" of the device.
 *
 * \ingroup pico
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the eeprom
 * \ingroup fraise_eeprom
 * 
 * \note This actually copies the flash area to RAM, so that it can be accessed in both read and write modes.
 */
void eeprom_setup();

/**
 * Read a byte
 * \ingroup fraise_eeprom
 * \param address the address to read
 * \return the data at this address
 */
char eeprom_user_read(int address);

/**
 * Write a byte
 * \ingroup fraise_eeprom
 * \param address the address to write to
 * \param data the value of the byte to write
 */
void eeprom_user_write(int address, char data);

/**
 * Change the device name in eeprom
 * \ingroup fraise_eeprom
 * \param newname the new name to store (no more than 16 chars)
 */
void eeprom_write_name(char *newname);

/**
 * Read the device name stored in eeprom
 * \ingroup fraise_eeprom
 * \return the currently stored name
 */
const char *eeprom_get_name();

/**
 * Change the device ID in eeprom
 * \ingroup fraise_eeprom
 * \param newid the new ID to store (must be in range [1:126])
 */
void eeprom_set_id(uint8_t newid);

/**
 * Read the device ID stored in eeprom
 * \ingroup fraise_eeprom
 * \return the currently stored ID
 */
uint8_t eeprom_get_id();

/**
 * Actually write the eeprom area to flash
 * \ingroup fraise_eeprom
 */
void eeprom_commit();

#ifdef __cplusplus
}
#endif

#endif // _FRAISE_EEPROM_H
