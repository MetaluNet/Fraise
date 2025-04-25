/**
 * Copyright (c) 2023 metalu.net
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "dcmotor.h"
#include <stdlib.h>

#define PWM_MAX 5555 // 22.5kHz
//#define PWM_MAX 10000 // 12.5kHz
//#define PWM_MAX 20000 // 6.25kHz
void dcmotor_init(dcmotor_t *p, int pinA, int pinB, int pinPwm, int pinCurrent) {
    p->pinA = pinA;
    gpio_init(pinA);
    gpio_put(p->pinA, 0);
    gpio_set_dir(pinA, GPIO_OUT);

    p->pinB = pinB;
    gpio_init(pinB);
    gpio_put(p->pinB, 0);
    gpio_set_dir(pinB, GPIO_OUT);

    p->pinPwm = pinPwm;
    gpio_set_function(pinPwm, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pinPwm);
    pwm_set_wrap(slice_num, PWM_MAX);
    pwm_set_enabled(slice_num, true);
    pwm_set_gpio_level(pinPwm, 0);

    p->pinCurrent = pinCurrent;
    if(p->pinCurrent > 0) {
        adc_init();
        adc_gpio_init(pinCurrent);
    }

    p->pwm = 0;
}

void dcmotor_set_pwm(dcmotor_t *p, int16_t pwm){
    p->pwm = pwm;
    int pwm_real = ((int)pwm * PWM_MAX) / 32768;
    pwm_set_gpio_level(p->pinPwm, abs(pwm_real));
    //fraise_printf("l pwm:%d abs_pwm: %d\n", pwm_real, abs(pwm_real));
    if(pwm > 0) {
        gpio_put(p->pinA, 1);
        gpio_put(p->pinB, 0);
    } else {
        gpio_put(p->pinA, 0);
        gpio_put(p->pinB, 1);
    }
}

int dcmotor_get_current_mA(dcmotor_t *p) {
    if(p->pinCurrent <= 0) return 0;

    adc_select_input(p->pinCurrent - 26);
    int adc = adc_read();
    // adc = vsense * 4096 / 3.3
    // vsense = isense * 1k = imotor/1540 * 1000
    // imotor = vsense * 1540 / 1000 = (adc * 3.3 / 4096) * 1540 / 1000 = adc * (3.3 *  1540) / (4096 * 1000)
    // imotor_mA = adc * (3.3 *  1540) / 4096 = adc * 1.24
    return (adc * ((3.3 * 1540) / 4096));
}
