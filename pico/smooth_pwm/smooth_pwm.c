/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "smooth_pwm.h"

void smooth_pwm_init(smooth_pwm_t *p, int pin) {
    p->pin = pin;
    gpio_set_function(pin, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice_num, 65535);
    pwm_set_enabled(slice_num, true);

    pwm_set_gpio_level(pin, 0);
    p->smooth = 0.1;
    p->target = p->value = 0;
}
    
inline void smooth_pwm_goto(smooth_pwm_t *p, float target) {
    p->target = target;
}

inline void smooth_pwm_set(smooth_pwm_t *p, float value) {
    p->target = p->value = value;
    float val = p->value;
    val = val * val * 65535.0;
    pwm_set_gpio_level(p->pin, (int)val);
}

inline void smooth_pwm_set_smooth(smooth_pwm_t *p, float smooth) {
    p->smooth = smooth;
}

inline void smooth_pwm_tick(smooth_pwm_t *p) {
    p->value = p->value * (1.0 - p->smooth) + p->target * p->smooth;
    float val = p->value;
    val = val * val * 65535.0;
    pwm_set_gpio_level(p->pin, (int)val);
}

