/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "fraise_buffers.h"


// TX buffer
#define TXBUF_SIZE 512
static char txbuf[TXBUF_SIZE];
static int txbuf_write_head = 1;
static int txbuf_write_tmphead;
static int txbuf_write_len;
static int txbuf_write_checksum;

static int txbuf_read_head = 0;
static int txbuf_read_tmphead;
static int txbuf_read_len = 0;

static inline int txbuf_inc_head(int h) {
    h++;
    if(h == TXBUF_SIZE) h = 0;
    return h;
}

// Write a message to the TX buffer:

bool txbuf_write_init(){          // Init a new message; returns false if txbuf is full.
    int freespace = txbuf_read_head - txbuf_write_head;
    if(freespace < 0) freespace += TXBUF_SIZE;
    if(freespace < 34) return false;
    txbuf_write_tmphead = txbuf_inc_head(txbuf_write_head); // keep first byte for length byte
    txbuf_write_len = 0;
    txbuf_write_checksum = 0;
    return true;
}

void txbuf_write_putc(char c){    // Add byte to the message
    txbuf[txbuf_write_tmphead] = c;
    txbuf_write_tmphead = txbuf_inc_head(txbuf_write_tmphead);
    txbuf_write_len++;
    txbuf_write_checksum += c;
}

void txbuf_write_finish(bool isChar){ // Finish the message
    if(isChar) txbuf_write_len |= 128;
    txbuf[txbuf_write_head] = txbuf_write_len; // write length byte
    txbuf_write_checksum += txbuf_write_len;
    txbuf[txbuf_write_tmphead] = -txbuf_write_checksum; // write checksum byte
    txbuf_write_head = txbuf_inc_head(txbuf_write_tmphead);
}

// Write a message from the TX buffer:

uint8_t txbuf_read_init(){           // Initialize the sender for next message. Returns the length of the next message (0 if none).
    int usedspace = txbuf_write_head - txbuf_read_head - 1;
    if(usedspace < 0) usedspace += TXBUF_SIZE;
    if(usedspace == 0) return 0;
    txbuf_read_tmphead = txbuf_inc_head(txbuf_read_head);
    txbuf_read_len = (txbuf[txbuf_read_tmphead] & 31) + 2; // total_len = data_len + 1(length byte) + 1(checksum byte)
    return txbuf_read_len;
}

char txbuf_read_getc(){           // Get next byte to send
    if(txbuf_read_len == 0) return 0;
    char c = txbuf[txbuf_read_tmphead];
    txbuf_read_len--;
    if(txbuf_read_len) txbuf_read_tmphead = txbuf_inc_head(txbuf_read_tmphead);
    return c;
}

void txbuf_read_finish(){         // Signal that the message has been sent successfully
    txbuf_read_head = txbuf_read_tmphead;
}


#if 0 // We don't use RX buffer for now. The following code has NOT been tested neither debugged!

// RX buffer

#define RXBUF_SIZE 512
static char rxbuf[RXBUF_SIZE];
static int rxbuf_write_head = 1;
static int rxbuf_write_tmphead;
static int rxbuf_write_len;
static int rxbuf_read_head = 0;
static int rxbuf_read_tmphead;
static int rxbuf_read_len = 0;

static inline int rxbuf_inc_head(int h) {
    h++;
    if(h == RXBUF_SIZE) h = 0;
    return h;
}

// Write a message to the RX buffer:

bool rxbuf_write_init(){          // Init a new message; returns false if rxbuf is full.
    int freespace = rxbuf_read_head - rxbuf_write_head;
    if(freespace < 0) freespace += RXBUF_SIZE;
    if(freespace < 34) return false;
    rxbuf_write_tmphead = rxbuf_inc_head(rxbuf_write_head); // keep first byte for length byte
    rxbuf_write_len = 0;
    return true;
}

void rxbuf_write_putc(char c){    // Add byte to the message
    rxbuf[rxbuf_write_tmphead] = c;
    rxbuf_write_tmphead = rxbuf_inc_head(rxbuf_write_tmphead);
    rxbuf_write_len++;
}

void rxbuf_write_finish(bool isChar){ // Finish the message
    if(isChar) rxbuf_write_len |= 128;
    rxbuf[rxbuf_write_head] = rxbuf_write_len; // write length byte
    rxbuf_write_head = rxbuf_write_tmphead;
}

// Write a message from the RX buffer:

uint8_t rxbuf_read_init(){ // Start reading the next available next message. Returns the length of the message (0 if none).
    int usedspace = rxbuf_write_head - rxbuf_read_head - 1;
    if(usedspace < 0) usedspace += RXBUF_SIZE;
    if(usedspace == 0) return 0;
    rxbuf_read_tmphead = rxbuf_inc_head(rxbuf_read_head);
    rxbuf_read_len = (rxbuf[rxbuf_read_tmphead] & 31) + 1; // total_len = data_len + 1(length byte)
    return rxbuf_read_len;
}

char rxbuf_read_getc(){           // Get next byte to send
    if(rxbuf_read_len == 0) return 0;
    char c = rxbuf[rxbuf_read_tmphead];
    rxbuf_read_len--;
    if(rxbuf_read_len) rxbuf_read_tmphead = rxbuf_inc_head(rxbuf_read_tmphead);
    return c;
}

void rxbuf_read_finish(){         // Signal that the message has been sent successfully
    rxbuf_read_head = rxbuf_read_tmphead;
}
#endif

