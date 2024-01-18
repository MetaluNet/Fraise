/**
 * Copyright (c) 2024 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pico/stdio/driver.h"
#include "RP2040.h"

#define _FRAISE_INTERNAL_
#include "fraise.pio.h"
#include "fraise_master.h"
#include "fraise_eeprom.h"

static PIO pio;
static uint sm;
static uint pgm_offset; // Offset of the program in the pio
static int8_t pio_irq;  // The irq used by the sm
static uint irq_index;  // The offset of pio_irq relatively to PIO0_IRQ_0 or PIO1_IRQ_0

static uint irq_count, irq_rx_count; // Debugging counters
static bool is_bootloading = false;

// Find a free pio and state machine and load the program into it.
// Returns false if this fails.
static bool init_pio(const pio_program_t *program, PIO *pio_hw, uint *sm, uint *offset) {
    // Find a free pio
    *pio_hw = pio1;
    if (!pio_can_add_program(*pio_hw, program)) {
        *pio_hw = pio0;
        if (!pio_can_add_program(*pio_hw, program)) {
            *offset = -1;
            return false;
        }
    }
    *offset = pio_add_program(*pio_hw, program);
    // Find a state machine
    *sm = (int8_t)pio_claim_unused_sm(*pio_hw, false);
    if (*sm < 0) {
        return false;
    }
    return true;
}

void fraise_setup() {
}

void fraise_get_pins(int *rxpin, int *txpin, int *drvpin)
{
}

void fraise_unsetup() {
}

// -----------------------------

void fraise_master_set_poll(uint8_t id, bool poll){
	if(id == 0) {
		if(poll) printf("sC00\n");
		else printf("sc00\n");
	}
}

void fraise_master_reset_polls() {
}

// -----------------------------

void fraise_master_sendbytes(uint8_t id, const char *data, uint8_t len) {
	if(id == 0) {
		fraise_receivebytes(data, len);
		return;
	}
}

void fraise_master_sendchars(uint8_t id, const char *data){
	if(id == 0) {
		fraise_receivechars(data, strlen(data));
		return;
	}
}

void fraise_master_sendbytes_broadcast(const char *data, uint8_t len) {
	fraise_receivebytes_broadcast(data, len);
}

void fraise_master_sendchars_broadcast(const char *data) {
	fraise_receivechars_broadcast(data, strlen(data));
}

// -----------------------------

void fraise_master_assign(const char* fruitname, uint8_t id){
}

// -----------------------------

void fraise_master_start_bootload(const char *buf){
	is_bootloading = true;
}

void fraise_master_stop_bootload(){
	is_bootloading = false;
}

bool fraise_master_is_bootloading() {
	return is_bootloading;
}

void fraise_master_send_bootload(const char *buf){
}

// ------------------------------
// virtual fruit stdout emulation

void fraise_putchar(char c) {
	static char line[64];
	static int count = 0;
	if(c == '\n') {
		line[count] = 0;
		printf("80%s\n", line);
		count = 0;
		return;
	}
	if(count < 64) line[count++] = c;
}

void fraise_printf(char* fmt, ...) {
	va_list args;
	char buf[64];
	char *p = buf;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	//printf("80%s", buf);
	while(*p) fraise_putchar(*p++);
}


