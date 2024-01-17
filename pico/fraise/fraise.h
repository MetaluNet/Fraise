/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_H
#define _FRAISE_H

#include "pico/stdlib.h"
#include <stdio.h>

// Initialize Fraise driver
void fraise_setup();

// Send a text message (must be a null-terminated string)
bool fraise_puts(char* msg); // returns true on success
// Send a raw bytes message
bool fraise_putbytes(char* data, uint8_t len); // returns true on success

// Optional user defined callbacks
void setup();
void loop();
void fraise_receivebytes(char *data, uint8_t len);           // Called when a raw bytes message has been received
void fraise_receivechars(char *data, uint8_t len);           // Called when a text message has been received
void fraise_receivebytes_broadcast(char *data, uint8_t len); // Called when a raw bytes broadcast message has been received
void fraise_receivechars_broadcast(char *data, uint8_t len); // Called when a text broadcast message has been received

// Get the pin numbers that Fraise is using
void fraise_get_pins(int *rxpin, int *txpin, int *drvpin);

#endif // _FRAISE_H
