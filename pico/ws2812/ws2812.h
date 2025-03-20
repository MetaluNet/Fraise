/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WS2812_H
#define _WS2812_H

/** \file ws2812.h
 *  \defgroup ws2812 ws2812
 * ws2812 (aka pixel led) module.
 *  \ingroup pico
 */

#ifdef __cplusplus
extern "C" {
#endif

bool ws2812_setup(int pin, bool rgbw);

void ws2812_put_pixel(uint32_t pixel_rgb);

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 24) |
            ((uint32_t) (g) << 16) |
            ((uint32_t) (b) << 8 );
}

void ws2812_dma_transfer(uint32_t *pixels, int num);
int ws2812_print_status();
#ifdef __cplusplus
}
#endif

#endif
