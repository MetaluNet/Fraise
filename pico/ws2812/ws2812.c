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
#include "hardware/dma.h"

#include "fraise.h"
#include "ws2812.pio.h"


static PIO pio;
static uint sm;
static uint pgm_offset; // Offset of the program in the pio
//static uint pio_irq;  // The irq used by the sm
//static uint irq_index;  // The offset of pio_irq relatively to PIO0_IRQ_0 or PIO1_IRQ_0
int dma_chan = -1;
dma_channel_config dma_config;

void ws2812_put_pixel(uint32_t pixel_rgb) {
    pio_sm_put_blocking(pio, sm, pixel_rgb);
}

bool ws2812_setup(int pin, bool rgbw) {
    if (!claim_pio_sm_irq(&ws2812_program, &pio, &sm, &pgm_offset, NULL /*&pio_irq*/)) {
        //panic("failed to setup pio");
        return false;
    }
    ws2812_program_init(pio, sm, pgm_offset, pin, 800000, rgbw);
    return true;
}

void ws2812_dma_transfer(uint32_t *pixels, int num) {
    // Get a free channel, panic() if there are none
    if(dma_chan == -1) {
        dma_chan = dma_claim_unused_channel(true);
        dma_config = dma_channel_get_default_config(dma_chan);
        channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
        channel_config_set_read_increment(&dma_config, true);
        channel_config_set_write_increment(&dma_config, false);
        channel_config_set_dreq(&dma_config, pio_get_dreq(pio, sm, true));
    }

    dma_channel_configure(
        dma_chan,          // Channel to be configured
        &dma_config,            // The configuration we just created
        &pio->txf[sm], // The write address
        pixels,        // The initial read address
        num,           // Number of transfers
        true           // Start immediately.
    );
}

void ws2812_print_status() {
    printf("l ws psold %d %d %d %d %d\n", PIO_NUM(pio), sm, pgm_offset, ws2812_program.length, dma_chan);  // pio sm offset length dma_chan
}
