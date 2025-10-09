/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "pico/sem.h"

#include "fraise.h"
#include "ws2812par.h"
#include "ws2812par.pio.h"

static PIO pio;
static uint sm;
static uint pgm_offset; // Offset of the program in the pio
static int nb_strips = 0;
static int strip_length = MAX_NUM_PIXELS * 4;

// bit plane content dma channel
#define DMA_CHANNEL 0
// chain channel for configuring main dma channel to output from disjoint 8 word fragments of memory
#define DMA_CB_CHANNEL 1

#define DMA_CHANNEL_MASK (1u << DMA_CHANNEL)
#define DMA_CB_CHANNEL_MASK (1u << DMA_CB_CHANNEL)
#define DMA_CHANNELS_MASK (DMA_CHANNEL_MASK | DMA_CB_CHANNEL_MASK)

// posted when it is safe to output a new set of values
static struct semaphore reset_delay_complete_sem;
// alarm handle for handling delay
static alarm_id_t reset_delay_alarm_id;

static int64_t reset_delay_complete(__unused alarm_id_t id, __unused void *user_data) {
    reset_delay_alarm_id = 0;
    sem_release(&reset_delay_complete_sem);
    // no repeat
    return 0;
}

static void __isr dma_complete_handler() {
    if (dma_hw->ints0 & DMA_CHANNEL_MASK) {
        // clear IRQ
        dma_hw->ints0 = DMA_CHANNEL_MASK;
        // when the dma is complete we start the reset delay timer
        if (reset_delay_alarm_id) cancel_alarm(reset_delay_alarm_id);
        reset_delay_alarm_id = add_alarm_in_us(400, reset_delay_complete, NULL, true);
    }
}

static void dma_init(PIO pio, uint sm) {
    dma_claim_mask(DMA_CHANNELS_MASK);

    // main DMA channel outputs 8 word fragments, and then chains back to the chain channel
    dma_channel_config channel_config = dma_channel_get_default_config(DMA_CHANNEL);
    channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
    channel_config_set_chain_to(&channel_config, DMA_CB_CHANNEL);
    channel_config_set_irq_quiet(&channel_config, true);
    dma_channel_configure(DMA_CHANNEL,
                          &channel_config,
                          &pio->txf[sm],
                          NULL, // set by chain
                          8, // 8 words for 8 bit planes
                          false);

    // chain channel sends single word pointer to start of fragment each time
    dma_channel_config chain_config = dma_channel_get_default_config(DMA_CB_CHANNEL);
    dma_channel_configure(DMA_CB_CHANNEL,
                          &chain_config,
                          &dma_channel_hw_addr(
                                  DMA_CHANNEL)->al3_read_addr_trig,  // ch DMA config (target "ring" buffer size 4) - this is (read_addr trigger)
                          NULL, // set later
                          1,
                          false);

    irq_set_exclusive_handler(DMA_IRQ_0, dma_complete_handler);
    dma_channel_set_irq0_enabled(DMA_CHANNEL, true);
    irq_set_enabled(DMA_IRQ_0, true);
}

// we store value (8 bits of a single color (R/G/B/W) value) for multiple
// strips of pixels, in bit planes. bit plane N has the Nth bit of each strip of pixels.
typedef struct {
    // stored MSB first
    uint32_t planes[8];
} value_bits_t;

// start of each value fragment (+1 for NULL terminator)
static uintptr_t fragment_start[MAX_NUM_PIXELS * 4 + 1];

static void output_strips_dma(value_bits_t *bits, uint value_length) {
    for (uint i = 0; i < value_length; i++) {
        fragment_start[i] = (uintptr_t) bits[i].planes; // MSB first
    }
    fragment_start[value_length] = 0;
    dma_channel_hw_addr(DMA_CB_CHANNEL)->al3_read_addr_trig = (uintptr_t) fragment_start;
}

// takes 8 bit color values and store in bit planes
static void transform_strips(strip_t **strips, uint num_strips, value_bits_t *bits, uint value_length) {
    for (uint v = 0; v < value_length; v++) {
        memset(&bits[v], 0, sizeof(bits[v]));
        for (uint i = 0; i < num_strips; i++) {
            uint32_t value = strips[i]->data[(v + strips[i]->data_offset) % value_length];
            for (int j = 0; j < 8 && value; j++, value >>= 1u) {
                if (value & 1u) bits[v].planes[8 - 1 - j] |= 1u << i;
            }
        }
    }
}

// ----------------- public functions ---------------

bool ws2812par_setup(int pin, int nstrips, int length, bool invert) {
    nb_strips = 0;
    if(length > MAX_NUM_PIXELS * 4) return false;
    if(!invert) {
        bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_parallel_program, &pio, &sm, &pgm_offset, pin, nstrips, true);
        if(!success) return false;
        ws2812_parallel_program_init(pio, sm, pgm_offset, pin, nstrips, 800000);
    } else {
        bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_parallel_inv_program, &pio, &sm, &pgm_offset, pin, nstrips, true);
        if(!success) return false;
        ws2812_parallel_inv_program_init(pio, sm, pgm_offset, pin, nstrips, 800000);
    }
    sem_init(&reset_delay_complete_sem, 1, 1); // initially posted so we don't block first time
    dma_init(pio, sm);
    nb_strips = nstrips;
    strip_length = length;
    return true;
}

// double buffer the state of the pixel strip, since we update next version in parallel with DMAing out old version
static value_bits_t states[2][MAX_NUM_PIXELS * 4 + 1];
static uint current = 0;

void ws2812par_process_strips(strip_t **strips) {
    if(nb_strips == 0) return;
    transform_strips(strips, nb_strips, states[current], strip_length);
}

void ws2812par_dma_transfer() {
    if(nb_strips == 0) return;
    sem_acquire_blocking(&reset_delay_complete_sem);
    output_strips_dma(states[current], strip_length);
    current ^= 1;
}

bool ws2812par_dma_ready() {
    return sem_available(&reset_delay_complete_sem) > 0;
}

void ws2812par_print_status() {
    if(nb_strips == 0) printf("l wspar configuration error!");
    else printf("l wspar psol %d %d %d %d\n", PIO_NUM(pio), sm, pgm_offset, ws2812_parallel_program.length); // pio sm offset length
}
