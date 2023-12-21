/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SMOOTH_PWM_H
#define _SMOOTH_PWM_H

typedef struct {
    int pin;
    float smooth; // 0.0 - 1.0
    float value;
    float target;
} smooth_pwm_t;

void smooth_pwm_init(smooth_pwm_t *p, int pin);
void smooth_pwm_goto(smooth_pwm_t *p, float target);
void smooth_pwm_set(smooth_pwm_t *p, float value);
void smooth_pwm_set_smooth(smooth_pwm_t *p, float smooth);
void smooth_pwm_tick(smooth_pwm_t *p);

#endif
