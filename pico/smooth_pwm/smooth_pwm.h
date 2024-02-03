/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SMOOTH_PWM_H
#define _SMOOTH_PWM_H

typedef struct _smooth_pwm_t {
    int pin;
    float smooth; 
    float value;  // 0.0 - 1.0
    float target; // 0.0 - 1.0
} smooth_pwm_t;

void smooth_pwm_init(smooth_pwm_t *p, int pin);             // bound a smooth_pwm to a GPIO pin
void smooth_pwm_goto(smooth_pwm_t *p, float target);        // set the target value (0.0 - 1.0)
void smooth_pwm_set(smooth_pwm_t *p, float value);          // instantly set the value (0.0 - 1.0)
void smooth_pwm_set_smooth(smooth_pwm_t *p, float smooth);  // set the smooth factor (0.0 - 1.0)
void smooth_pwm_tick(smooth_pwm_t *p);                      // update the pwm value

#endif
