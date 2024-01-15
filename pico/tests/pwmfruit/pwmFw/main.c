/**
 * 12-channel PWM driver
 */

#define BOARD pico

#include <stdio.h>
#include <string.h>
#include "fraise_device.h"
#include "smooth_pwm.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

#define NUM_PWM 12
smooth_pwm_t pwms[NUM_PWM];

void setup() {
	smooth_pwm_init(&pwms[0], 3);
	smooth_pwm_init(&pwms[1], 4);
	smooth_pwm_init(&pwms[2], 5);
	smooth_pwm_init(&pwms[3], 8);
	smooth_pwm_init(&pwms[4], 9);
	smooth_pwm_init(&pwms[5], 10);
	smooth_pwm_init(&pwms[6], 11);
	smooth_pwm_init(&pwms[7], 12);
	smooth_pwm_init(&pwms[8], 13);
	smooth_pwm_init(&pwms[9], 14);
	smooth_pwm_init(&pwms[10], 15);
	smooth_pwm_init(&pwms[11], 22);
}

void loop(){
	static absolute_time_t next;// = make_timeout_time_ms(100);
	static absolute_time_t nextpwm;
	static bool led = false;

	fraise_poll_rx();

	if(absolute_time_min(nextpwm, get_absolute_time()) == nextpwm) {
		for(int i = 0; i < NUM_PWM; i++) smooth_pwm_tick(&pwms[i]);
		nextpwm = make_timeout_time_ms(5);
	}

	if(absolute_time_min(next, get_absolute_time()) == next) {
		gpio_put(LED_PIN, led = !led);
		next = make_timeout_time_ms(100);
	}
}

void fraise_receivebytes(char *data, uint8_t len){
	//printf("receive bytes: "); for(int i = 0; i < len; i++) printf("%d ", (uint8_t)data[i]); putchar('\n');
	if(data[0] == 10 && len > 3) {
		int p = data[1];
		int count = 2;
		while(count + 1 < len) {
			float val = (((int)data[count] << 8) + data[count + 1]) / 65536.0;
			//val = (val * val) * 65535;
			if(p < NUM_PWM) smooth_pwm_goto(&pwms[p], val);
			//printf("pwm[%d]=%f : d0=%d d1=%d\n", p, val, data[count], data[count + 1]);
			p++;
			count += 2;
		}
	}
	else if(data[0] == 11 && len > 3) {
		int p = data[1];
		int count = 2;
		while(count + 1 < len) {
			float smooth = (((int)data[count] << 8) + data[count + 1]) / 65536.0;
			if(p < NUM_PWM) smooth_pwm_set_smooth(&pwms[p], smooth);
			p++;
			count += 2;
		}
	}
	else if(data[0] == 12 && len > 2) {
			float val = (((int)data[1] << 8) + data[2]) / 65536.0;
			//val = (val * val) * 65535;
			for(int i = 0; i < NUM_PWM; i++) smooth_pwm_goto(&pwms[i], val);
	}
	else if(data[0] == 13 && len > 2) {
			float smooth = (((int)data[1] << 8) + data[2]) / 65536.0;
			for(int i = 0; i < NUM_PWM; i++) smooth_pwm_set_smooth(&pwms[i], smooth);
	}

}

void fraise_receivechars(char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

