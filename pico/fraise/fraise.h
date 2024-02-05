/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_H
#define _FRAISE_H

#include "pico/stdlib.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize Fraise driver
void fraise_setup();

// Send a text message (must be a null-terminated string)
bool fraise_puts(const char* msg); // returns true on success
// Send a raw bytes message
bool fraise_putbytes(const char* data, uint8_t len); // returns true on success

// Optional user defined callbacks
void setup();
void loop();
void fraise_receivebytes(const char *data, uint8_t len);           // Called when a raw bytes message has been received
void fraise_receivechars(const char *data, uint8_t len);           // Called when a text message has been received
void fraise_receivebytes_broadcast(const char *data, uint8_t len); // Called when a raw bytes broadcast message has been received
void fraise_receivechars_broadcast(const char *data, uint8_t len); // Called when a text broadcast message has been received

// Get the pin numbers that Fraise is using
void fraise_get_pins(int *rxpin, int *txpin, int *drvpin);

void fraise_printf(const char* fmt, ...) __attribute__ ((format (printf, 1, 2)));
void fraise_putchar(char c);

#ifndef FRAISE_DONT_OVERWRITE_PRINTF
#define printf fraise_printf
#define putchar fraise_putchar
#endif

#ifdef __cplusplus
}
#endif

#endif // _FRAISE_H
