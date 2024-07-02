/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_H
#define _FRAISE_H

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/pio.h"

/** \file fraise.h
 *  \defgroup fraise fraise
 * Common Fraise API
 *  \ingroup pico
 *
 */
///@{

#ifdef __cplusplus
extern "C" {
#endif

/** -----------------------------------------------------------------
 * \name Sending raw bytes messages
 * \{
 */

/**
 * \brief Send a raw bytes message
 * \param data the address of the first byte to send
 * \param len the number of bytes to send
 * \return false if the TX buffer is full, true otherwise
 */
bool fraise_putbytes(const char* data, uint8_t len); // returns true on success

void fraise_put_init(); /**< \brief Initialize the 'put' buffer */
void fraise_put_bytes(const char* data, uint8_t len);
void fraise_put_int8(int8_t b);
void fraise_put_uint8(uint8_t b);
void fraise_put_int16(int16_t b);
void fraise_put_uint16(uint16_t b);
void fraise_put_int32(int32_t b);
void fraise_put_uint32(uint32_t b);
bool fraise_put_send(); /**< \brief Send the 'put' buffer with fraise_putbytes() */
///@}

/** -----------------------------------------------------------------
 * \name Sending text messages
 * \{
 */

/**
 * \brief Send a text message
 * \param msg the text message to send, must be a null-terminated string
 * \return false if the TX buffer is full, true otherwise
 */
bool fraise_puts(const char* msg); // returns true on success

/**
 * \brief push a single character to the text buffer
 * \param c the char to send
 * \note the message will be sent with fraise_puts() and flushed after each `\n` (newline) character
 */
void fraise_putchar(char c);
/**
 * \brief Push a printf-formatted buffered text to the text buffer
 * \param fmt,... the formatted text
 * \note the message will be sent and flushed after each `\n` character (fraise_printf() internally calls fraise_putchar())
 */
void fraise_printf(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));

///@}

/** -----------------------------------------------------------------
 * \name Optional user defined callbacks
 * \{
*/

/**
 * \brief Setup the application (optional user defined)
 * \note the user can define this function, which will be automatically called once at startup
 */
void setup();
/**
 * \brief Application loop (optional user defined)
 * \note the user can define this function, which will be called repeatedly after setup()
 */
void loop();
/**
 * \brief Receive raw bytes (optional user defined)
 * \param data the buffer containing the received bytes
 * \param len the number of received bytes
 * \note the user can define this function, which will be called when a raw bytes message has been received
 */
void fraise_receivebytes(const char *data, uint8_t len);           // Called when a raw bytes message has been received
/**
 * \brief Receive text message (optional user defined)
 * \param data the buffer containing the received characters
 * \param len the number of received characters
 * \note the user can define this function, which will be called when a text message has been received
 */
void fraise_receivechars(const char *data, uint8_t len);           // Called when a text message has been received
/**
 * \brief Receive broadcast raw bytes (optional user defined)
 * \param data the buffer containing the received bytes
 * \param len the number of received bytes
 * \note the user can define this function, which will be called when a broadcast raw bytes message has been received
 */
void fraise_receivebytes_broadcast(const char *data, uint8_t len); // Called when a raw bytes broadcast message has been received
/**
 * \brief Receive broadcast text message (optional user defined)
 * \param data the buffer containing the received characters
 * \param len the number of received characters
 * \note the user can define this function, which will be called when a broadcast text message has been received
 */
void fraise_receivechars_broadcast(const char *data, uint8_t len); // Called when a text broadcast message has been received

/** -----------------------------------------------------------------
 * \name Bytes receive decoding utils:
 * \note These functions are only valid when called from fraise_receivebytes() or fraise_receivebytes_broadcast().
 * If there's not enough data left, they return 0.
*/
int8_t fraise_get_int8();
uint8_t fraise_get_uint8();
int16_t fraise_get_int16();
uint16_t fraise_get_uint16();
int32_t fraise_get_int32();
uint32_t fraise_get_uint32();

/**
 * \brief Init the 'get' buffer.
 * \param data the buffer containing the data to decode
 * \param len the number of data bytes
 * \note this function is automatically called before fraise_receivebytes() or fraise_receivebytes_broadcast() are called.
 */
void fraise_init_get_buffer(const char *data, uint8_t len);

///@}

/** -----------------------------------------------------------------
 * \name Global fraise driver management
 * \{
*/

/**
 * \brief Initialize the Fraise driver
 * \note This is automatically called at startup, so in most cases you won't need it.
 */
void fraise_setup();

/**
 * \brief Stop and cleanup the Fraise driver
 */
void fraise_unsetup();

/**
 * \brief Get the pin numbers that Fraise is using
 * \param rxpin the address where to store the number of the RX pin
 * \param txpin the address where to store the number of the TX pin
 * \param drvpin the address where to store the number of the DRV pin
 */
 void fraise_get_pins(int *rxpin, int *txpin, int *drvpin);

///@}

/** -----------------------------------------------------------------
 * \name Misc
 * \{
*/

/**
 * \brief Request attribution of a pio/sm (and optionnaly irq) for a given pio program
 * \param program the pio program we want a sm to run
 * \param pio_hw the address where to store the available pio
 * \param sm the address where to store the index of the available state machine
 * \param program_offset the address where to store the offset from which the program can be written
 * \param irq if not null, the address where to store the index of the chosen irq
 * \return false if this fails.
 */
bool claim_pio_sm_irq(const pio_program_t *program, PIO *pio_hw, uint *sm, uint *program_offset, uint *irq);

///@}

#ifndef FRAISE_DONT_OVERWRITE_PRINTF
#define printf fraise_printf
#define putchar fraise_putchar
#endif

#ifdef __cplusplus
}
#endif

///@}

#endif // _FRAISE_H
