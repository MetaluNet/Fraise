/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DC_MOTOR_H
#define _DC_MOTOR_H

/** \file dcmotor.h
 *  \defgroup dcmotor dcmotor
 * DC motor control module
 *  \ingroup pico
 */

typedef struct _dcmotor_t {
    int pinA, pinB, pinPwm, pinCurrent;
    int16_t pwm;
} dcmotor_t;

void dcmotor_init(dcmotor_t *p, int pinA, int pinB, int pinPwm, int pinCurrent);
void dcmotor_set_pwm(dcmotor_t *p, int16_t pwm);        // pwm [-32768, 32767]
int dcmotor_get_current_mA(dcmotor_t *p);

#endif
