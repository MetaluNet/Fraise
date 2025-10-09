/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WS2812PAR_H
#define _WS2812PAR_H

/** \file ws2812par.h
 *  \defgroup ws2812par ws2812par
 * parallel ws2812 (aka pixel led) module.
 * transfer up to 32 ws2811 strings in parallel
 *  \ingroup pico
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_NUM_PIXELS
#define MAX_NUM_PIXELS 300
#endif

typedef struct {
    uint8_t *data;
    uint data_offset; // start at data_offset index, then read modulo(strip_length)
} strip_t;

bool ws2812par_setup(int pin, int nb_strips, int strip_length, bool invert); // 'strip_length' is in bytes (maximum: MAX_NUM_PIXELS * 4)

void ws2812par_process_strips(strip_t **strips); // strips data must be at least 'strip_length' long (in bytes)

bool ws2812par_dma_ready(); // return true if the dma is ready to transfer

void ws2812par_dma_transfer(); // block until dma is ready

void ws2812par_print_status();

#ifdef __cplusplus
}
#endif

#endif
