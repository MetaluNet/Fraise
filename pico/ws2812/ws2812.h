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

void ws2812_setup(bool rgbw);

void ws2812_put_pixel(uint32_t pixel_grb);

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

#endif
