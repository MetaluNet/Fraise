/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define BOARD pico

#include "pico/stdlib.h"
#include "ws2812par.h"
#include "fraise.h"
#include "cpuload.h"

#define WS2812_PIN_START 0

#ifndef NUM_PIXELS
#define NUM_PIXELS 100
#endif

static uint8_t strips_data[17][NUM_PIXELS * 3];
static uint8_t *framebuffer = strips_data[0];

strip_t strip0 = {
    .data = strips_data[0],
    .data_offset = 0,
};

strip_t strip1 = {
    .data = strips_data[1],
    .data_offset = 0,
};

strip_t *strips[2] = {
    &strip0,
    &strip1,
};

int fb_index;

void set_pixel(int n, uint8_t r, uint8_t g, uint8_t b) {
    if(n >= 0 && n < NUM_PIXELS) {
        framebuffer[n * 3 + 0] = r;
        framebuffer[n * 3 + 1] = g;
        framebuffer[n * 3 + 2] = b;
    }
}

void put_pixel(uint32_t pixel_rgb) {
    if(fb_index < NUM_PIXELS) {
        framebuffer[fb_index * 3 + 0] = pixel_rgb >> 24;
        framebuffer[fb_index * 3 + 1] = pixel_rgb >> 16;
        framebuffer[fb_index * 3 + 2] = pixel_rgb >> 8;
        fb_index++;
    }
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
        (((uint32_t) (r) << 24) |
         ((uint32_t) (g) << 16) |
         ((uint32_t) (b) << 8 ));
}

void pattern_snakes(uint len, uint t) {
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 0)) % 64;
        if (x < 10)
            put_pixel(urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            put_pixel(urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            put_pixel(urgb_u32(0, 0, 0xff));
        else
            put_pixel(0);
    }
}

void pattern_random(uint len, uint t) {
    for (uint i = 0; i < len; ++i)
        put_pixel(rand());
}

void pattern_sparkle(uint len, uint t) {
    for (uint i = 0; i < len; ++i)
        put_pixel(rand() % 16 ? 0 : 0xffffffff);
}

void pattern_greys(uint len, uint t) {
    uint max = 100; // let's not draw too much current!
    t %= max;
    for (uint i = 0; i < len; ++i) {
        put_pixel(t * 0x10101);
        if (++t >= max) t = 0;
    }
}

typedef void (*pattern)(uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
    {pattern_snakes,  "Snakes!"},
    {pattern_random,  "Random data"},
    {pattern_sparkle, "Sparkles"},
    {pattern_greys,   "Greys"},
};

//-------------------------------------------//

int pat;
int dir;
int t = 0;
int i = 1000;
bool pattern_play = false;
bool pattern_rnd = false;
int rotate_pos;
int rotate_speed;

CpuLoad pat_cpuload("pattern");
CpuLoad wsp_cpuload("wsprocess");
CpuLoad dma_cpuload("dma");

#define ROTATE_BITS 16
static int last_offset;

void process()
{
    pat_cpuload.start();
    if(pattern_play) {
        if(i++ >= 1000) {
            if(pattern_rnd) {
                pat = rand() % count_of(pattern_table);
                dir = (rand() >> 30) & 1 ? 1 : -1;
            }
            i = 0;
        }
        fb_index = 0;
        pattern_table[pat].pat(NUM_PIXELS, t);
        t += dir;
    }
    pat_cpuload.stop();

    rotate_pos += rotate_speed;
    if(rotate_pos < 0) rotate_pos += (NUM_PIXELS << ROTATE_BITS);
    else if(rotate_pos >= (NUM_PIXELS << ROTATE_BITS)) rotate_pos -= (NUM_PIXELS << ROTATE_BITS);
    int offset = (rotate_pos >> ROTATE_BITS) * 3;
    strips[0]->data_offset = (rotate_pos >> ROTATE_BITS) * 3;
    if(last_offset != offset) {
        last_offset = offset;
        //fraise_printf("l offset %d\n", offset);
    }

    wsp_cpuload.start();
    ws2812par_process_strips(strips);
    wsp_cpuload.stop();
}

void transfer()
{
    dma_cpuload.start();
    ws2812par_dma_transfer();
    dma_cpuload.stop();
}


void setup() {
    ws2812par_setup(WS2812_PIN_START, count_of(strips), NUM_PIXELS * 3);
    process();
}

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
int ledPeriod = 250;
int transfer_count = 0;
int fps = 100000; // fastest as possible

void loop() {
    static absolute_time_t nextLed;
    static bool led = false;
    static absolute_time_t nextFrame;

    if(time_reached(nextLed)) {
        gpio_put(LED_PIN, led = !led);
        nextLed = make_timeout_time_ms(ledPeriod);
    }
    if(fps > 0 && time_reached(nextFrame) && ws2812par_dma_ready()) {
        nextFrame = make_timeout_time_us(1e6 / fps);
        transfer();
        process();
        transfer_count++;
    }
}

void fraise_receivebytes(const char* data, uint8_t len) {
    char command = fraise_get_int8();
    switch(command) {
    case 10: 
        fps = fraise_get_uint16();
        if(fps < 0) fps = 0;
        break;
    case 20:
        set_pixel(fraise_get_int16(), fraise_get_int8(), fraise_get_int8(), fraise_get_int8());
        break;
    case 21:
        pattern_play = fraise_get_int8() != 0;
        pattern_rnd = fraise_get_int8() != 0;
        pat = fraise_get_int8();
        dir = fraise_get_int8() != 0 ? 1 : -1;
        break;
    case 30:
        rotate_speed = fraise_get_int32();
        break;
    case 31:
        rotate_pos = 0;
        break;
    case 100:
        transfer();
        process();
        break;
    case 101:
        pat_cpuload.get_load();
        wsp_cpuload.get_load();
        dma_cpuload.get_load();
        break;
    case 102:
        pat_cpuload.reset();
        wsp_cpuload.reset();
        dma_cpuload.reset();
        break;
    case 103:
        fraise_printf("l transfers %d\n", transfer_count);
        transfer_count = 0;
        break;
    }
}

void fraise_receivechars(const char *data, uint8_t len) {
    if(data[0] == 'E') { // Echo
        fraise_printf("E%s\n", data + 1);
    }
}

