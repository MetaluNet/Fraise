/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_H
#define _FRAISE_H

#include "pico/stdlib.h"
#include <stdio.h>

/** \file fraise.h
 *  \defgroup fraise fraise
 *
 * Common Fraise API
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the Fraise driver
 * \ingroup fraise
 * 
 * \note This is automatically called at startup, so in most cases you won't need it.
 */
void fraise_setup();

/**
 * Stop and cleanup the Fraise driver
 * \ingroup fraise
 */
void fraise_unsetup();


// ------------- Message sending utilities ----------------

/**
 * Send a text message
 * \ingroup fraise
 * \param msg the text message to send, must be a null-terminated string
 * \return false if the TX buffer is full, true otherwise
 */
bool fraise_puts(const char* msg); // returns true on success

/**
 * Send a raw bytes message
 * \ingroup fraise
 * \param data the address of the first byte to send
 * \param len the number of bytes to send
 * \return false if the TX buffer is full, true otherwise
 */
bool fraise_putbytes(const char* data, uint8_t len); // returns true on success

/**
 * push a single character to the buffer
 * \ingroup fraise
 * \param c the char to send
 * \note the message will be sent and flushed after each '\n' character
 */
void fraise_putchar(char c);
/**
 * Push a printf-formatted buffered text to the buffer
 * \ingroup fraise
 * \param fmt,... the formatted text
 * \note the message will be sent and flushed after each '\n' character (fraise_printf() internally uses fraise_putchar())
 */
void fraise_printf(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));


// ------------- Optional user defined callbacks ----------------

/**
 * Setup the application (optional user defined)
 * \ingroup fraise
 * \note the user can define this function, which will be automatically called once at startup
 */
void setup();
/**
 * Application loop (optional user defined)
 * \ingroup fraise
 * \note the user can define this function, which will be called repeatedly after setup()
 */
void loop();
/**
 * Receive raw bytes (optional user defined)
 * \ingroup fraise
 * \param data the buffer containing the received bytes
 * \param len the number of received bytes
 * \note the user can define this function, which will be called when a raw bytes message has been received
 */
void fraise_receivebytes(const char *data, uint8_t len);           // Called when a raw bytes message has been received
/**
 * Receive text message (optional user defined)
 * \ingroup fraise
 * \param data the buffer containing the received characters
 * \param len the number of received characters
 * \note the user can define this function, which will be called when a text message has been received
 */
void fraise_receivechars(const char *data, uint8_t len);           // Called when a text message has been received
/**
 * Receive broadcast raw bytes (optional user defined)
 * \ingroup fraise
 * \param data the buffer containing the received bytes
 * \param len the number of received bytes
 * \note the user can define this function, which will be called when a broadcast raw bytes message has been received
 */
void fraise_receivebytes_broadcast(const char *data, uint8_t len); // Called when a raw bytes broadcast message has been received
/**
 * Receive broadcast text message (optional user defined)
 * \ingroup fraise
 * \param data the buffer containing the received characters
 * \param len the number of received characters
 * \note the user can define this function, which will be called when a broadcast text message has been received
 */
void fraise_receivechars_broadcast(const char *data, uint8_t len); // Called when a text broadcast message has been received


// Get the pin numbers that Fraise is using
void fraise_get_pins(int *rxpin, int *txpin, int *drvpin);


#ifndef FRAISE_DONT_OVERWRITE_PRINTF
#define printf fraise_printf
#define putchar fraise_putchar
#endif

#ifdef __cplusplus
}
#endif

#endif // _FRAISE_H
