/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "fraise.h"

#include "ws2812.pio.h"

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 2
#endif

static PIO pio;
static uint sm;
static uint pgm_offset; // Offset of the program in the pio
//static uint pio_irq;  // The irq used by the sm
//static uint irq_index;  // The offset of pio_irq relatively to PIO0_IRQ_0 or PIO1_IRQ_0

void ws2812_put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

void ws2812_setup(bool rgbw) {
    if (!claim_pio_sm_irq(&ws2812_program, &pio, &sm, &pgm_offset, NULL /*&pio_irq*/)) {
        panic("failed to setup pio");
    }
    ws2812_program_init(pio, sm, pgm_offset, WS2812_PIN, 800000, rgbw);
}

