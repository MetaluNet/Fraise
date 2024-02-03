/**
 * Example project for the smooth_pwm module.
 */

#define BOARD pico

#include <stdio.h>
#include <string.h>
#include "fraise.h"
#include "smooth_pwm.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
smooth_pwm_t LedPWM;
repeating_timer_t pwm_tick_timer;

bool pwm_tick_callback(repeating_timer_t *rt)
{
	smooth_pwm_tick(&LedPWM);
	return true;
}

void setup() {
	smooth_pwm_init(&LedPWM, LED_PIN);
	add_repeating_timer_ms(5, pwm_tick_callback, NULL, &pwm_tick_timer);
}

void fraise_receivebytes(const char *data, uint8_t len){
	if(data[0] == 10) {
		smooth_pwm_goto(&LedPWM, (((int)data[1] << 8) + data[2]) / 65535.0);
	}
	else if(data[0] == 11) {
		smooth_pwm_set_smooth(&LedPWM, (((int)data[1] << 8) + data[2]) / 65535.0);
	}
}

void fraise_receivechars(const char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

