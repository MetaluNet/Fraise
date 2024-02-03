/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "fraise_master_buffers.h"

// ------------------------------------------------------------------------------------
// ------------ TX buffer ------------ 
// The message is encoded like this:
// data_length data data (...) checksum (the first 'data' byte is the destination fruit ID)
// data_length is the number of data bytes, so the whole message takes (data_length&63 + 2) bytes in txbuf.
// data_length = number_of_data_bytes + 128 * message_is_char

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

// ------ Write a message to the TX buffer: ------ 

// Init a new message; returns false if txbuf is full.
bool txbuf_write_init(int len){
    int freespace = txbuf_read_head - txbuf_write_head;
    if(freespace < 0) freespace += TXBUF_SIZE;
    if(freespace < len + 2) return false;
    txbuf_write_tmphead = txbuf_inc_head(txbuf_write_head); // keep first byte for length byte
    txbuf_write_len = 0;
    txbuf_write_checksum = 0;
    return true;
}

// Add byte to the message
void txbuf_write_putc(char c){
    txbuf[txbuf_write_tmphead] = c;
    txbuf_write_tmphead = txbuf_inc_head(txbuf_write_tmphead);
    txbuf_write_len++;
    txbuf_write_checksum += c;
}

// Validate the message
void txbuf_write_finish(){
    txbuf[txbuf_write_head] = txbuf_write_len; // write length byte
    txbuf[txbuf_write_tmphead] = -txbuf_write_checksum; // write checksum byte
    txbuf_write_head = txbuf_inc_head(txbuf_write_tmphead);
}

// ------ Read a message from the TX buffer:

// Initialize the sender for next message. Returns the length of the next message (0 if none).
uint8_t txbuf_read_init(){
    int usedspace = txbuf_write_head - txbuf_read_head - 1;
    if(usedspace < 0) usedspace += TXBUF_SIZE;
    if(usedspace == 0) return 0;
    txbuf_read_tmphead = txbuf_inc_head(txbuf_read_head);
    txbuf_read_len = (txbuf[txbuf_read_tmphead] & 63) + 2; // total_len = data_len + 1(length byte) + 1(checksum byte)
    return txbuf_read_len;
}

// Get next byte to send
char txbuf_read_getc(){
    if(txbuf_read_len == 0) return 0;
    char c = txbuf[txbuf_read_tmphead];
    txbuf_read_len--;
    if(txbuf_read_len) txbuf_read_tmphead = txbuf_inc_head(txbuf_read_tmphead);
    return c;
}

// Signal that the message has been successfully sent
void txbuf_read_finish(){
    txbuf_read_head = txbuf_read_tmphead;
}


// ------------------------------------------------------------------------------------
// ------------ RX buffer ------------ 

// The message is encoded like this:
// data_length data data (...) (the first 'data' byte is the fruit ID which sends the message)
// data_length&63 is the number of data bytes, so the whole message takes (data_length&63 + 1) bytes in rxbuf.
// data_length = number_of_data_bytes + 128 * message_is_char

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

// ------ Write a message to the RX buffer:

// Init a new message; returns false if rxbuf is full.
bool rxbuf_write_init(){
    int freespace = rxbuf_read_head - rxbuf_write_head;
    if(freespace < 0) freespace += RXBUF_SIZE;
    if(freespace < 34) {
        //printf("e rxbuf_write_init() full!!!\n");
        return false;
    }
    rxbuf_write_tmphead = rxbuf_inc_head(rxbuf_write_head); // keep first byte for length byte
    rxbuf_write_len = 0;
    return true;
}

// Add byte to the message
void rxbuf_write_putc(char c){
    rxbuf[rxbuf_write_tmphead] = c;
    rxbuf_write_tmphead = rxbuf_inc_head(rxbuf_write_tmphead);
    rxbuf_write_len++;
}

// Validate the message
void rxbuf_write_finish(bool isChar){
    if(isChar) rxbuf_write_len |= 128;
    rxbuf[rxbuf_write_head] = rxbuf_write_len; // write length byte
    rxbuf_write_head = rxbuf_write_tmphead;
}

// ------ Read a message from the RX buffer:

// Start reading the next available next message. Returns the length of the message (0 if none).
uint8_t rxbuf_read_init(){
    int usedspace = rxbuf_write_head - rxbuf_read_head - 1;
    if(usedspace < 0) usedspace += RXBUF_SIZE;
    if(usedspace == 0) return 0;
    rxbuf_read_tmphead = rxbuf_inc_head(rxbuf_read_head);
    char l = rxbuf[rxbuf_read_tmphead];
    rxbuf_read_len = (l & 63) ;
    rxbuf_read_tmphead = rxbuf_inc_head(rxbuf_read_tmphead); // skip length byte
    return l; // return length_byte = number_of_data_bytes + 128*isChar
}

// Get next byte to send
char rxbuf_read_getc(){
    if(rxbuf_read_len == 0) return 0;
    char c = rxbuf[rxbuf_read_tmphead];
    rxbuf_read_len--;
    if(rxbuf_read_len) rxbuf_read_tmphead = rxbuf_inc_head(rxbuf_read_tmphead);
    return c;
}

// Signal that the message has been read
void rxbuf_read_finish(){
    rxbuf_read_head = rxbuf_read_tmphead;
}

// ------------------------------------------------------------------------------------
// ------ Reset buffers:
void fraise_master_buffers_reset() {
	rxbuf_write_head = 1;
	rxbuf_read_head = 0;
	txbuf_write_head = 1;
	txbuf_read_head = 0;
}



