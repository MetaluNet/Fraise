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
 * Eeprom is first initialized at boot time by the system calling eeprom_setup(), 
 * which copies the last saved version of the data from flash to an dedicated RAM area.
 * This area can then be read or modified.
 * If modified, the RAM area must be written back to flash with eeprom_commit().
 *
 * The fraise_eeprom API reserves some bytes at the beginning of the emulated EEPROM to store fraise system data,
 * such as the "name" and the "ID" of the device.
 *
 * \ingroup pico
 */
///@{

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \name Hi-level persistent parameter API
 * This API allows to declare a set of parameters, and automates saving to and reloading from the EEPROM emulation.
*/
///@{

/**
 * \brief User defined function where to declare the parameters.
 * \note User has to define this function, which must be a list of eeprom_declare_*() calls.
 * Example:
 * \code{.c}
 * void eeprom_main() {
 *     eeprom_declare_uint8(&my_uint8);
 *     eeprom_declare_int16(&my_int16);
 * }
 * \endcode
 */
extern void eeprom_declare_main();

/**
 * \brief Declare data of arbitrary size.
 * \note use in eeprom_declare_main() only
 * \param data the address of the declared data
 * \param size the size of the data
 */
void eeprom_declare_data(char *data, uint8_t size);
/**
 * \brief Declare a 8-bit signed parameter.
 * \note use in eeprom_declare_main() only
 * \param data the address of the declared parameter
 */
inline void eeprom_declare_int8(int8_t *data)		{ eeprom_declare_data((char*)data, 1); }
/**
 * \brief Declare a 8-bit unsigned parameter.
 * \note use in eeprom_declare_main() only
 * \param data the address of the declared parameter
 */
inline void eeprom_declare_uint8(uint8_t *data)		{ eeprom_declare_data((char*)data, 1); }
/**
 * \brief Declare a 16-bit signed parameter.
 * \note use in eeprom_declare_main() only
 * \param data the address of the declared parameter
 */
inline void eeprom_declare_int16(int16_t *data)		{ eeprom_declare_data((char*)data, 2); }
/**
 * \brief Declare a 16-bit unsigned parameter.
 * \note use in eeprom_declare_main() only
 * \param data the address of the declared parameter
 */
inline void eeprom_declare_uint16(uint16_t *data)	{ eeprom_declare_data((char*)data, 2); }
/**
 * \brief Declare a 32-bit signed parameter.
 * \note use in eeprom_declare_main() only
 * \param data the address of the declared parameter
 */
inline void eeprom_declare_int32(int32_t *data)		{ eeprom_declare_data((char*)data, 4); }
/**
 * \brief Declare a 32-bit unsigned parameter.
 * \note use in eeprom_declare_main() only
 * \param data the address of the declared parameter
 */
inline void eeprom_declare_uint32(uint32_t *data)	{ eeprom_declare_data((char*)data, 4); }
/**
 * \brief Reload all the parameters from eeprom
 * \note You might want to call this function in your setup().
 */
void eeprom_load();
/**
 * \brief Save all the parameters to eeprom
 * \note eeprom_save() automatically calls eeprom_commit() after the RAM image is updated.
 */
void eeprom_save();
///@}


/**
 * \name Low-level API
 */
///@{

/**
 * \brief Initialize the eeprom by copying data from flash to RAM
 * \note This function is automatically called at startup. You shouldn't need to call it manually.
 */
void eeprom_setup();

/**
 * \brief Read a byte from eeprom user space
 * \param address the address to read
 * \return the data at this address
 */
char eeprom_user_read(int address);

/**
 * \brief Write a byte to eeprom user space
 * \param address the address to write to
 * \param data the value of the byte to write
 */
void eeprom_user_write(int address, char data);

/**
 * \brief Change the device name in eeprom
 * \param newname the new name to store (no more than 16 chars)
 */
void eeprom_write_name(char *newname);

/**
 * \brief Read the device name stored in eeprom
 * \return the currently stored name
 */
const char *eeprom_get_name();

/**
 * \brief Change the device ID in eeprom
 * \param newid the new ID to store (must be in range [1:126])
 */
void eeprom_set_id(uint8_t newid);

/**
 * \brief Read the device ID stored in eeprom
 * \return the currently stored ID
 */
uint8_t eeprom_get_id();

/**
 * \brief Actually write the eeprom area to flash
 * \note This function is protected with a critical section, which ensures
 * that interrupt are fully disabled during actual flash writing.
 */
void eeprom_commit();

///@}

///@}

#ifdef __cplusplus
}
#endif

#endif // _FRAISE_EEPROM_H
