/**
 * Example project for the smooth_pwm module.
 */

#define BOARD pico

#include <stdio.h>
#include <string.h>
#include "fraise.h"
#include "dcmotor.h"

#define PIN_A       15
#define PIN_B        9
#define PIN_PWM     11
#define PIN_SEL     13
#define PIN_CURRENT 26

/*#define PIN_A       7
#define PIN_B        9
#define PIN_PWM     3
#define PIN_CURRENT 26
#define PIN_SEL     13
*/

/*#define PIN_A       7
#define PIN_B       1
#define PIN_PWM     3
#define PIN_CURRENT 27*/

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
dcmotor_t motor;

void setup() {
	dcmotor_init(&motor, PIN_A, PIN_B, PIN_PWM, PIN_CURRENT);
    gpio_init(PIN_SEL);
    gpio_set_dir(PIN_SEL, GPIO_OUT);
    gpio_put(PIN_SEL, 0);
}


int ledPeriod = 250;

void loop(){
	static absolute_time_t nextLed;
	static bool led = false;

	if(time_reached(nextLed)) {
		gpio_put(LED_PIN, led = !led);
		nextLed = make_timeout_time_ms(ledPeriod);
	}
}

void fraise_receivebytes(const char *data, uint8_t len){
	int command = fraise_get_uint8();
	if(command == 1) ledPeriod = fraise_get_uint8() * 10;
	else if(command == 10) {
		int pwm = fraise_get_int16();
		dcmotor_set_pwm(&motor, pwm);
		gpio_put(PIN_SEL, pwm > 0);
	}
	else if(command == 11) {
	    printf("c %d\n", dcmotor_get_current_mA(&motor));
	}
}

void fraise_receivechars(const char *data, uint8_t len){
	if(data[0] == 'E') { // Echo
		printf("E%s\n", data + 1);
	}
}

