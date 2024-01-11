/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FRAISE_DEVICE_H
#define _FRAISE_DEVICE_H

// Initialize Fraise driver
void fraise_setup();

// Process incoming messages.
// Call this function often (at least every few ms) to avoid overflowing the RX buffer.
void fraise_poll_rx();

void fraise_setID(uint8_t id);
void fraise_unsetup();

// Send a text message
bool fraise_puts(char* msg); // returns true on success
// Send a raw bytes message
bool fraise_putbytes(char* data, uint8_t len); // returns true on success

// Optional user defined callbacks:
void fraise_receivebytes(char *data, uint8_t len);              // Called when a raw bytes message has been received
void fraise_receivechars(char *data, uint8_t len);              // Called when a text message has been received
void fraise_receivebytes_broadcast(char *data, uint8_t len);    // Called when a raw bytes broadcast message has been received
void fraise_receivechars_broadcast(char *data, uint8_t len);    // Called when a text broadcast message has been received

void fraise_debug_print_next_txmessage(); // print next pending message to send, and remove it from the list
uint fraise_debug_get_irq_count();
uint fraise_debug_get_irq_rx_count();

#endif // _FRAISE_DEVICE_H