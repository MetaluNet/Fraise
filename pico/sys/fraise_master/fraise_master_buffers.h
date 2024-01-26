/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FRAISE_MASTER_BUFFER_H
#define _FRAISE_MASTER_BUFFER_H

bool    txbuf_write_init();                 // Init a new message; returns false if txbuf is full.
void    txbuf_write_putc(char c);           // Add byte to the message.
void    txbuf_write_finish();               // Validate the message.

uint8_t txbuf_read_init();                  // Initialize the sender for next message. Returns the length of the next message (0 if none).
char    txbuf_read_getc();                  // Get next byte to send.
void    txbuf_read_finish();                // Remove the message from the buffer.


bool    rxbuf_write_init();                 // Init a new message; returns false if rxbuf is full.
void    rxbuf_write_putc(char c);           // Add byte to the message.
void    rxbuf_write_finish(bool isChar);    // Validate the message.

uint8_t rxbuf_read_init();                  // Start reading the next available message. Returns the length of the message (0 if none), 
                                            // plus 128 if it's a "char" one.
char    rxbuf_read_getc();                  // Get next byte of the message.
void    rxbuf_read_finish();                // Remove the message from the buffer.

void    fraise_master_buffers_reset();

#endif // _FRAISE_MASTER_BUFFER_H

