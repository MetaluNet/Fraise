/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#define _FRAISE_INTERNAL_
#include "fraise_master.h"
#include "fraise_master_buffers.h"
#include "fraise_master_private.h"

#if 0
// -----------------------------

void fraise_master_set_poll(uint8_t id, bool poll){
	if(id == 0) {
		if(poll) printf("sC00\n");
		else printf("sc00\n");
	} else if(id < MAX_FRUITS) {
		if(poll) set_polled(id);
		else clear_polled(id);
		clear_detected(id);
	}
}

void fraise_master_reset_polls() {
	for(int id = 0; id < MAX_FRUITS ; id++) {
		clear_polled(id);
		clear_detected(id);
	}
}

// -----------------------------

void fraise_master_sendbytes_raw(uint8_t id, const char *data, uint8_t len, bool isChar) {
	if(id >= MAX_FRUITS) return;
	if(!txbuf_write_init()) {
		printf("e TXBUF full!\n");
		return;
	}
	txbuf_write_putc(id);
	txbuf_write_putc(len | (128 * isChar));
	for(int i = 0; i < len; i++) txbuf_write_putc(data[i]);
	txbuf_write_finish();
}

void fraise_master_sendbytes(uint8_t id, const char *data, uint8_t len) {
	if(id == 0) {
		fraise_receivebytes(data, len);
		return;
	}
	fraise_master_sendbytes_raw(id, data, len, false);
}

void fraise_master_sendchars(uint8_t id, const char *data){
	int len = strlen(data);
	if(id == 0) {
		fraise_receivechars(data, len);
		return;
	}
	fraise_master_sendbytes_raw(id, data, len, true);
}

void fraise_master_sendbytes_broadcast(const char *data, uint8_t len) {
	fraise_receivebytes_broadcast(data, len);
	fraise_master_sendbytes_raw(0, data, len, false);
}

void fraise_master_sendchars_broadcast(const char *data) {
	int len = strlen(data);
	fraise_receivechars_broadcast(data, len);
	fraise_master_sendbytes_raw(0, data, len, true);
}

// -----------------------------

void fraise_master_service() {
	int l;
	while((l = rxbuf_read_init())) {
		rxbuf_read_finish();
	}
}
#endif
