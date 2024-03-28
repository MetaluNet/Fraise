/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "fraise.h"

/* -------------------------------------------------*/
/* ------- Message receive "get" functions ---------*/
struct {
    const char *buf;
    uint8_t len;
} get_buffer;

void fraise_init_get_buffer(const char *buf, uint8_t len){
    get_buffer.buf = buf;
    get_buffer.len = len;
}

int8_t fraise_get_int8(){
    if(get_buffer.len >= 1) {
        int8_t ret = (int8_t)(get_buffer.buf[0]);
        get_buffer.buf++;
        get_buffer.len--;
        return ret;
    } else return 0;
}

uint8_t fraise_get_uint8(){
    if(get_buffer.len >= 1) {
        uint8_t ret = (uint8_t)(get_buffer.buf[0]);
        get_buffer.buf++;
        get_buffer.len--;
        return ret;
    } else return 0;
}

int16_t fraise_get_int16(){
    if(get_buffer.len >= 2) {
        int16_t ret = (int16_t)((get_buffer.buf[0] << 8) | get_buffer.buf[1]);
        get_buffer.buf += 2;
        get_buffer.len -= 2;
        return ret;
    } else {
        get_buffer.len = 0;
        return 0;
    }
}

uint16_t fraise_get_uint16() {
    if(get_buffer.len >= 2) {
        uint16_t ret = (uint16_t)((get_buffer.buf[0] << 8) | get_buffer.buf[1]);
        get_buffer.buf += 2;
        get_buffer.len -= 2;
        return ret;
    } else {
        get_buffer.len = 0;
        return 0;
    }
}

int32_t fraise_get_int32(){
    if(get_buffer.len >= 4) {
        int32_t ret = (int32_t)(
            (get_buffer.buf[0] << 24) | (get_buffer.buf[1] << 16) |
            (get_buffer.buf[2] << 8) | get_buffer.buf[3]
        );
        get_buffer.buf += 4;
        get_buffer.len -= 4;
        return ret;
    } else {
        get_buffer.len = 0;
        return 0;
    }
}

uint32_t fraise_get_uint32(){
    if(get_buffer.len >= 4) {
        uint32_t ret = (uint32_t)(
            (get_buffer.buf[0] << 24) | (get_buffer.buf[1] << 16) |
            (get_buffer.buf[2] << 8) | get_buffer.buf[3]
        );
        get_buffer.buf += 4;
        get_buffer.len -= 4;
        return ret;
    } else {
        get_buffer.len = 0;
        return 0;
    }
}

/* -------------------------------------------------*/
/* --------- Message send "put" functions ----------*/

struct {
    char buf[64];
    uint8_t len;
} put_buffer;

void fraise_put_init() { put_buffer.len = 0; }
void fraise_put_int8(int8_t b){
    if(put_buffer.len > sizeof(put_buffer.buf) - 1) return;
    put_buffer.buf[put_buffer.len++] = (char)b;
}
void fraise_put_uint8(uint8_t b){
    if(put_buffer.len > sizeof(put_buffer.buf) - 1) return;
    put_buffer.buf[put_buffer.len++] = (char)b;
}
void fraise_put_int16(int16_t b){
    if(put_buffer.len > sizeof(put_buffer.buf) - 2) return;
    put_buffer.buf[put_buffer.len++] = (char)(b >> 8);
    put_buffer.buf[put_buffer.len++] = (char)b;
}
void fraise_put_uint16(uint16_t b){
    if(put_buffer.len > sizeof(put_buffer.buf) - 2) return;
    put_buffer.buf[put_buffer.len++] = (char)(b >> 8);
    put_buffer.buf[put_buffer.len++] = (char)b;
}
void fraise_put_int32(int32_t b){
    if(put_buffer.len > sizeof(put_buffer.buf) - 4) return;
    put_buffer.buf[put_buffer.len++] = (char)(b >> 24);
    put_buffer.buf[put_buffer.len++] = (char)(b >> 16);
    put_buffer.buf[put_buffer.len++] = (char)(b >> 8);
    put_buffer.buf[put_buffer.len++] = (char)b;
}
void fraise_put_uint32(uint32_t b){
    if(put_buffer.len > sizeof(put_buffer.buf) - 4) return;
    put_buffer.buf[put_buffer.len++] = (char)(b >> 24);
    put_buffer.buf[put_buffer.len++] = (char)(b >> 16);
    put_buffer.buf[put_buffer.len++] = (char)(b >> 8);
    put_buffer.buf[put_buffer.len++] = (char)b;
}
bool fraise_put_send(){
    return fraise_putbytes(put_buffer.buf, put_buffer.len);
}


/* -------------------------------------------------*/
/* ------------------- Misc ------------------------*/

bool claim_pio_sm_irq(const pio_program_t *program, PIO *pio_hw, uint *sm, uint *program_offset, uint *irq) {
    // Find a free pio
    *pio_hw = pio1;
    if (!pio_can_add_program(*pio_hw, program)) {
        *pio_hw = pio0;
        if (!pio_can_add_program(*pio_hw, program)) {
            *program_offset = -1;
            return false;
        }
    }
    *program_offset = pio_add_program(*pio_hw, program);

    // Find a state machine
    *sm = (int8_t)pio_claim_unused_sm(*pio_hw, false);
    if (*sm < 0) {
        return false;
    }

    if(irq != NULL) {
		// Find a free irq
		static_assert(PIO0_IRQ_1 == PIO0_IRQ_0 + 1 && PIO1_IRQ_1 == PIO1_IRQ_0 + 1, "");
		uint pio_irq = (*pio_hw == pio0) ? PIO0_IRQ_0 : PIO1_IRQ_0;
		if (irq_get_exclusive_handler(pio_irq)) {
		    pio_irq++;
		    if (irq_get_exclusive_handler(pio_irq)) {
		        return false;
		    }
		}
		*irq = pio_irq;
    }

    return true;
}

